#pragma once

#include "Core/Core.h"
#include "Job.h"
#include "JobAllocator.h"
#include "Utility/MPMCQueue.h"
#include "Utility/WorkStealingQueue.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace plagc
{
    class JobSystem;

    class ThreadWrapper
    {
    public:
        ThreadWrapper(void(JobSystem::*func)(std::size_t), JobSystem* instance,
                      std::size_t index)
            : m_Thread(func, instance, index) {}

        ThreadWrapper(ThreadWrapper&& other) noexcept
            : m_Thread(std::move(other.m_Thread)) {}

        ThreadWrapper& operator=(ThreadWrapper&& other) noexcept
        {
            if (this != &other)
            {
                Join();
                m_Thread = std::move(other.m_Thread);
            }
            return *this;
        }

        ThreadWrapper(const ThreadWrapper&) = delete;
        ThreadWrapper& operator=(const ThreadWrapper&) = delete;

        void Join()
        {
            if (m_Thread.joinable())
                m_Thread.join();
        }

        ~ThreadWrapper() { Join(); }

    private:
        std::thread m_Thread;
    };

    class JobGraph
    {
    public:
        explicit JobGraph(JobSystem& jobSystem) : r_JobSystem(jobSystem) {}
        ~JobGraph();
        JobGraph(const JobGraph&) = delete;
        JobGraph& operator=(const JobGraph&) = delete;

        template<typename F>
        Job* AddJob(F&& fn);

        template<typename F>
        Job* ParallelFor(std::size_t start, std::size_t end, F&& fn);

        void AddDependency(Job* dependent, Job* independent);
        void Submit();

    private:
        std::vector<Job*> m_JobList;
        JobSystem& r_JobSystem;
        bool m_Submitted = false;
    };

    class JobSystem
    {
    public:
        JobSystem();
        ~JobSystem();

        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        // Blocks until all in-flight work and cancellation processing finishes.
        // Must be invoked by the owning/main thread, not by a worker.
        // External submission and graph-building must have stopped beforehand.
        void ShutDown(bool valid = true);
        bool IsStopping() const noexcept
        {
            return m_ShouldClose.load(std::memory_order_acquire);
        }
        std::uint64_t OutstandingJobs() const noexcept
        {
            return m_OutstandingJobs.load(std::memory_order_acquire);
        }

        template<typename F>
        Job* CreateJob(F&& func, Job* parent = nullptr)
        {
            // Internal allocation: callable jobs may still spawn children while
            // shutdown has been requested. The worker will cancel those children.
            Job* job = JobAllocator::Allocate(std::forward<F>(func), parent);
            m_OutstandingJobs.fetch_add(1, std::memory_order_release);
            return job;
        }

        // Called AFTER deleting a job; necessary so shutdown never observes zero
        // before the last destructor has finished.
        void OnJobDestroyed() noexcept
        {
            m_OutstandingJobs.fetch_sub(1, std::memory_order_acq_rel);
        }

        template<typename F>
        void Submit(F&& fn)
        {
            if (IsStopping())
                throw std::runtime_error("JobSystem is shutting down");
            PushJob(CreateJob(std::forward<F>(fn)));
        }

        template<typename F>
        Job* ParallelFor(std::size_t start, std::size_t end, F&& fn)
        {
            if (IsStopping())
                throw std::runtime_error("JobSystem is shutting down");
            Job* root = parallel_for(start, end, std::forward<F>(fn));
            if (root)
                PushJob(root);
            return root; // DANGEROUS to dereference after asynchronous completion.
        }

        template<typename F>
        Job* parallel_for(std::size_t start, std::size_t end, F&& fn);

        template<typename F>
        Job* DivideJob(std::size_t start, std::size_t end,
                       std::size_t batchSize, const F* conquer, Job* parent);

        void PushJob(Job* job);
        std::size_t GetPoolSize() const noexcept { return m_ThreadPool.size(); }

    private:
        void OnDetach(Job* job);
        void Try_Work(std::size_t workerIndex);
        bool TrySteal(std::size_t workerIndex, Job*& job) noexcept;

        // Initialized on each worker thread; never used by external submitters.
        static thread_local JobSystem* s_CurrentSystem;
        static thread_local std::size_t s_CurrentWorker;

        // External submissions use MPMC; workers produce into their OWN deques.
        MPMCQueue<Job*> m_JobQueue{4096};
        std::vector<std::unique_ptr<WorkStealingQueue<Job*>>> m_LocalQueues;
        std::vector<ThreadWrapper> m_ThreadPool;
        std::atomic<bool> m_ShouldClose{false};
        std::atomic<std::uint64_t> m_OutstandingJobs{0};
    };

    template<typename F>
    Job* JobGraph::AddJob(F&& fn)
    {
        if (m_Submitted || r_JobSystem.IsStopping())
            throw std::runtime_error("Cannot add jobs to submitted/stopping graph");
        Job* job = r_JobSystem.CreateJob(std::forward<F>(fn));
        try { m_JobList.push_back(job); }
        catch (...) { JobAllocator::DeAllocate(job); r_JobSystem.OnJobDestroyed(); throw; }
        return job;
    }

    template<typename F>
    Job* JobGraph::ParallelFor(std::size_t start, std::size_t end, F&& fn)
    {
        if (m_Submitted || r_JobSystem.IsStopping())
            throw std::runtime_error("Cannot add jobs to submitted/stopping graph");
        Job* root = r_JobSystem.parallel_for(start, end, std::forward<F>(fn));
        if (!root)
            return nullptr;
        try { m_JobList.push_back(root); }
        catch (...) { JobAllocator::DeAllocate(root); r_JobSystem.OnJobDestroyed(); throw; }
        return root;
    }

    template<typename F>
    Job* JobSystem::parallel_for(std::size_t start, std::size_t end, F&& fn)
    {
        if (start >= end)
            return nullptr;
        const std::size_t partitions = m_ThreadPool.size();
        const std::size_t batchSize = (end - start + partitions - 1) / partitions;

        Job* root = CreateJob([] {}, nullptr);
        auto task = [this, start, end, batchSize,
                     Conquer = std::forward<F>(fn), root]()
        {
            if (end - start > batchSize)
            {
                const auto mid = start + (end - start) / 2;
                DivideJob(start, mid, batchSize, &Conquer, root);
                DivideJob(mid, end, batchSize, &Conquer, root);
            }
            else
            {
                for (std::size_t i = start; i < end; ++i)
                    Conquer(i);
            }
        };
        try { *root = std::move(task); }
        catch (...)
        {
            JobAllocator::DeAllocate(root);
            OnJobDestroyed();
            throw;
        }
        return root;
    }

    template<typename F>
    Job* JobSystem::DivideJob(std::size_t start, std::size_t end,
                              std::size_t batchSize, const F* conquer, Job* parent)
    {
        // An active parent can still create children as it finishes during
        // shutdown; those children are counted and will be cancelled in OnDetach.
        Job* child = CreateJob([] {}, parent);
        try
        {
            *child = [this, start, end, batchSize, conquer, child]()
            {
                if (end - start > batchSize)
                {
                    const auto mid = start + (end - start) / 2;
                    DivideJob(start, mid, batchSize, conquer, child);
                    DivideJob(mid, end, batchSize, conquer, child);
                }
                else
                {
                    for (std::size_t i = start; i < end; ++i)
                        (*conquer)(i);
                }
            };
        }
        catch (...)
        {
            // Child was never published. Undo its parent registration.
            parent->m_UnfinishedJobs.fetch_sub(1, std::memory_order_acq_rel);
            JobAllocator::DeAllocate(child);
            OnJobDestroyed();
            throw;
        }
        PushJob(child);
        return child;
    }
}
