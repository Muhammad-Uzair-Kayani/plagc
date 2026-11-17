#include "pch.h"
#include "JobSystem.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace
{
    std::uint32_t WorkerCount()
    {
        const auto n = std::thread::hardware_concurrency();
        return n == 0 ? 4u : std::max(1u, n - 2);
    }
}

namespace plagc
{
    thread_local JobSystem* JobSystem::s_CurrentSystem = nullptr;
    thread_local std::size_t JobSystem::s_CurrentWorker = 0;

    JobGraph::~JobGraph()
    {
        // Only an UNSUBMITTED graph owns its job objects. Once submitted,
        // JobSystem completion owns their lifetime (graph may die immediately).
        if (!m_Submitted)
        {
            for (Job* job : m_JobList)
            {
                JobAllocator::DeAllocate(job);
                r_JobSystem.OnJobDestroyed();
            }
        }
    }

    void JobGraph::AddDependency(Job* dependent, Job* independent)
    {
        if (m_Submitted)
            throw std::runtime_error("Cannot change dependencies after Submit");
        if (!dependent || !independent || dependent == independent)
            throw std::invalid_argument("Invalid JobGraph dependency");
        // Graph construction must be single-threaded and completed before Submit.
        independent->AddContinuation(dependent);
        dependent->m_DependencyCount.fetch_add(1, std::memory_order_relaxed);
    }

    void JobGraph::Submit()
    {
        if (m_Submitted || r_JobSystem.IsStopping())
            throw std::runtime_error("Graph already submitted or system stopping");

        // Validate the complete DAG BEFORE publishing any root. Otherwise a
        // cycle with some ready roots could leave unqueued jobs alive forever.
        std::unordered_map<Job*, std::size_t> index;
        index.reserve(m_JobList.size());
        std::vector<std::uint32_t> indegree;
        indegree.reserve(m_JobList.size());
        std::vector<Job*> ready;
        ready.reserve(m_JobList.size());
        for (std::size_t i = 0; i < m_JobList.size(); ++i)
        {
            Job* job = m_JobList[i];
            if (!index.emplace(job, i).second)
                throw std::runtime_error("Duplicate job in graph");
            const auto n = job->m_DependencyCount.load(std::memory_order_relaxed);
            indegree.push_back(n);
            if (n == 0)
                ready.push_back(job);
        }

        std::vector<Job*> traversal = ready;
        for (std::size_t i = 0; i < traversal.size(); ++i)
        {
            for (Job* dependent : traversal[i]->m_Continuations)
            {
                const auto it = index.find(dependent);
                if (it == index.end())
                    throw std::runtime_error("Dependency points outside graph");
                auto& n = indegree[it->second];
                if (n == 0)
                    throw std::runtime_error("Invalid dependency count");
                if (--n == 0)
                    traversal.push_back(dependent);
            }
        }
        if (traversal.size() != m_JobList.size())
            throw std::runtime_error("JobGraph contains a cycle or missing dependency");

        m_Submitted = true;
        for (Job* job : ready)
            r_JobSystem.PushJob(job);
    }

    JobSystem::JobSystem()
    {
        const auto count = WorkerCount();
        CORE_TRACE("Total Worker Initialized {0}", count);
        // Construct every local deque before starting any worker.
        m_LocalQueues.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i)
            m_LocalQueues.push_back(std::make_unique<WorkStealingQueue<Job*>>(1024));

        m_ThreadPool.reserve(count);
        try
        {
            for (std::uint32_t i = 0; i < count; ++i)
                m_ThreadPool.emplace_back(&JobSystem::Try_Work, this, i);
        }
        catch (...)
        {
            // If thread creation fails, let previously started workers exit.
            m_ShouldClose.store(true, std::memory_order_release);
            throw;
        }
    }

    JobSystem::~JobSystem()
    {
        ShutDown(true);
    }

    void JobSystem::ShutDown(bool valid)
    {
        if (!valid)
            return; // Shutdown is one-way; false does not restart threads.

        m_ShouldClose.store(true, std::memory_order_release);
        // Workers stay alive while *any* registered job remains (including
        // jobs blocked on prerequisites). Queued jobs are cancelled by OnDetach.
        for (auto& worker : m_ThreadPool)
            worker.Join();
    }

    void JobSystem::PushJob(Job* job)
    {
        if (!job)
            return;

        job->m_State.store(JobState::Ready, std::memory_order_relaxed);

        // Only the owning worker may push to its local work-stealing deque.
        if (s_CurrentSystem == this &&
            m_LocalQueues[s_CurrentWorker]->TryPush(job))
            return;

        // External callers and local-overflow fallback use the global MPMC.
        // Never silently lose a job if that queue becomes full.
        Job* pending = job;
        while (!m_JobQueue.EnQueue(std::move(pending)))
        {
            if (s_CurrentSystem != this)
            {
                // External submitters cannot pop from a worker-owned deque.
                std::this_thread::yield();
                continue;
            }

            Job* helping = nullptr;
            if (m_LocalQueues[s_CurrentWorker]->TryPop(helping) ||
                m_JobQueue.Try_Pop(helping) ||
                TrySteal(s_CurrentWorker, helping))
            {
                OnDetach(helping);
            }
            else
                std::this_thread::yield();
        }
    }

    bool JobSystem::TrySteal(std::size_t workerIndex, Job*& job) noexcept
    {
        const std::size_t count = m_LocalQueues.size();
        for (std::size_t offset = 1; offset < count; ++offset)
        {
            const std::size_t victim = (workerIndex + offset) % count;
            if (m_LocalQueues[victim]->TrySteal(job))
                return true;
        }
        return false;
    }

    void JobSystem::OnDetach(Job* job)
    {
        if (IsStopping() || job->m_CancelRequested.load(std::memory_order_acquire))
        {
            job->m_State.store(JobState::Terminated, std::memory_order_release);
        }
        else
        {
            try { job->Execute(); }
            catch (...) {
                // A failed callable is treated as cancelled; propagate to its
                // dependents. Exception reporting can be added separately.
                job->m_State.store(JobState::Terminated, std::memory_order_release);
            }
        }

        // The 'own work' reference is always decremented exactly once.
        // Children keep this Job alive until the last child completes.
        if (job->m_UnfinishedJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
            job->Finish(*this); // May delete job.
    }

    void JobSystem::Try_Work(std::size_t workerIndex)
    {
        s_CurrentSystem = this;
        s_CurrentWorker = workerIndex;

        for (;;)
        {
            Job* job = nullptr;
            if (m_LocalQueues[workerIndex]->TryPop(job) ||
                m_JobQueue.Try_Pop(job) ||
                TrySteal(workerIndex, job))
            {
                OnDetach(job);
                continue;
            }
            if (IsStopping() && OutstandingJobs() == 0)
                break;
            std::this_thread::yield();
        }

        s_CurrentSystem = nullptr;
    }
}
