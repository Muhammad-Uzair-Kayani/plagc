#pragma once

#include "Core/Core.h"
#include "Job.h"
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

		JobGraph() = default;

		void AddDependency(Job*, Job*);

	private:


	};

	class JobSystem
	{
	public:

		JobSystem();

		void ShutDown(bool valid) { m_ShouldClose.store(valid, std::memory_order_release); }

		void Submit(Job* _job);


	private:

		void Try_Work();

	private:

		MPMCQueue<Job*> m_JobQueue{4096};
		std::vector<ThreadWrapper> m_ThreadPool;
		std::atomic<bool> m_ShouldClose{ false };

	};
}