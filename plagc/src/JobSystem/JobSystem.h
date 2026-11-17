#pragma once

#include "Core/Core.h"
#include "Job.h"
#include "JobAllocator.h"
#include "Utility/MPMCQueue.h"

#include <vector>
#include <thread>
#include <atomic>
#include <exception>

namespace plagc
{
	class JobSystem;

	class ThreadWrapper
	{
	public:

		
		ThreadWrapper(void(JobSystem::*func)(), JobSystem* instance) : m_Thread(func, instance) {}

		ThreadWrapper(ThreadWrapper&& other) noexcept : m_Thread(std::move(other.m_Thread)) {}

		ThreadWrapper& operator=(ThreadWrapper&& other)
		{
			if (this == &other)
				throw std::runtime_error("Unable To Copy Self Thread Wrapper");

			if (m_Thread.joinable())
				m_Thread.join();

			m_Thread = std::move(other.m_Thread);

			return *this;
		}

		~ThreadWrapper()
		{
			if (m_Thread.joinable())
				m_Thread.join();
		}

	private:

		std::thread m_Thread;

	};

	class JobGraph
	{
	public:

		JobGraph(JobSystem& jobSystem) : r_JobSystem(jobSystem) {}

		template<typename F>
		Job* AddJob(F&& fn)
		{
			auto job = JobAllocator::Allocate(std::move(fn));
			m_JobList.push_back(job);
			return job;
		}

		template<typename F>
		Job* ParallelFor(size_t start, size_t end, F&& fn);

		void AddDependency(Job*, Job*);

		
		void Submit();

	private:

		std::vector<Job*> m_JobList;
		JobSystem& r_JobSystem;
	};

	class JobSystem
	{
	public:

		JobSystem();

		void ShutDown(bool valid) { m_ShouldClose.store(valid, std::memory_order_release); }

		template<typename F>
		void Submit(F&& fn)
		{
			auto job = JobAllocator::Allocate(std::move(fn));
			PushJob(job);
		}

		template<typename F>
		Job* ParallelFor(size_t start, size_t end, F&& fn)
		{
			auto job = Parallel_For(start, end, std::forward<F>(fn));
			
			if(job)
				PushJob(std::move(job));
			
			return job;
		}

		template<typename F>
		Job* parallel_for(size_t start, size_t end, F&& fn);

		void PushJob(Job* job);

		size_t GetPoolSize() const noexcept { return m_ThreadPool.size(); }

		template<typename F>
		Job* DivideJob(size_t start, size_t end, size_t batchSize, const F* p_Conquer, Job* job);

	private:

		void OnDetach(Job* job);

		void Try_Work();

	private:

		MPMCQueue<Job*> m_JobQueue{4096};
		std::vector<ThreadWrapper> m_ThreadPool;
		std::atomic<bool> m_ShouldClose{ false };

	};

	template<typename F>
	Job* plagc::JobGraph::ParallelFor(size_t start, size_t end, F&& fn)
	{

		auto job = r_JobSystem.parallel_for(start, end, std::forward<F>(fn));

		if(job)
			m_JobList.push_back(job);

		return job;
	}


	template<typename F>
	Job* plagc::JobSystem::parallel_for(size_t start, size_t end, F&& fn)
	{

		if (start >= end)
			return nullptr;

		auto partitions = m_ThreadPool.size();

		//Excat partitions as number of Workers
		//Only rounds up if one partition is required extra
		auto batchSize = (end - start + partitions - 1) / partitions;

		auto root = JobAllocator::Allocate([] {}, nullptr);

		auto task = [this, start, end, batchSize, Conquer = std::forward<F>(fn), root]
			{
				//Divide
				if (end - start > batchSize)
				{

					auto mid = (start + end) / 2;

					//Left Side Divide
					Job* left = DivideJob(start, mid, batchSize, &Conquer, root);
					//Right Side Divide
					Job* right = DivideJob(mid, end, batchSize, &Conquer, root);

				}
				//Conquer
				else
				{
					for (size_t i = start; i < end; ++i)
						Conquer(i);
				}
			};

		*root = (std::move(task));

		return root;
	}

	template<typename F>
	Job* plagc::JobSystem::DivideJob(size_t start, size_t end, size_t batchSize, const F* p_Conquer, Job* parent)
	{
		
		//Initializing Job with empty lambda
		auto root = JobAllocator::Allocate([] {}, parent);

		*root = [this, start, end, batchSize, p_Conquer, root]
			{
				//Divide
				if (end - start > batchSize)
				{

					auto mid = (start + end) / 2;

					//Left Side Divide
					Job* left = DivideJob(start, mid, batchSize, p_Conquer, root);
					//Right Side Divide
					Job* right = DivideJob(mid, end, batchSize, p_Conquer, root);

				}
				//Conquer
				else
				{
					for (size_t i = start; i < end; ++i)
						(*p_Conquer)(i);
				}
			};

		PushJob(std::move(root));

		return root;
	}

}