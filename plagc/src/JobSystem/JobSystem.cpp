#include "pch.h"
#include "JobSystem.h"

#include <algorithm>

uint32_t WorkerCount()
{
	uint32_t count = std::thread::hardware_concurrency();

	if (count == 0)
		return 4u;

	return std::max(1u, count - 2);

}

namespace plagc
{
	
	void JobGraph::AddDependency(Job* A, Job* B)
	{
		//B is dependent of A, A => Will Decrease dependency count of B
		A->AddContinuation(B);
	}

	JobSystem::JobSystem()
	{
		
		auto count = WorkerCount();

		for (uint32_t i = 0; i < count; ++i)
			m_ThreadPool.emplace_back(ThreadWrapper(&JobSystem::Try_Work, this));

	}

	void JobSystem::Submit(Job* _job)
	{
		m_JobQueue.EnQueue(std::move(_job));
	}

	void JobSystem::Try_Work()
	{
		while (true)
		{

			if (m_ShouldClose.load(std::memory_order_acquire))
				break;

			Job* job;
			if (m_JobQueue.Try_Pop(job))
			{
				job->Execute();
			}
			else
			{
				std::this_thread::yield();
			}

		}
	}

}