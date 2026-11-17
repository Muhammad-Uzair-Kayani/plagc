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
	
	void JobGraph::AddDependency(Job* dependent, Job* independent)
	{
		//1 is dependent of 2, 2 => Will Decrease dependency count of 1
		independent->AddContinuation(dependent);
		//Increase the dependency count of B
		uint32_t count = dependent->m_DependencyCount.load(std::memory_order_relaxed);
		dependent->m_DependencyCount.store(count + 1, std::memory_order_relaxed);
	}

	void JobGraph::Submit()
	{
		for (auto& job : m_JobList)
			if (job->m_DependencyCount.load(std::memory_order_relaxed) == 0)
				r_JobSystem.PushJob(job);
	}

	JobSystem::JobSystem()
	{
		
		auto count = WorkerCount();
		CORE_TRACE("Total Worker Initialized {0}", count);
		for (uint32_t i = 0; i < count; ++i)
			m_ThreadPool.emplace_back(ThreadWrapper(&JobSystem::Try_Work, this));

	}

	void JobSystem::PushJob(Job* job)
	{
		job->m_State = JobState::Ready;
		if (m_JobQueue.EnQueue(std::move(job)))
			CORE_TRACE("Job Pushed Into Global Queue");
	}

	void JobSystem::OnDetach(Job* job)
	{
		if (m_ShouldClose.load(std::memory_order_acquire))
		{

			if (job->m_UnfinishedJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				job->m_State = JobState::Terminated;
				job->Finish(*this);
			}

			return;
		}

		CORE_TRACE("Worker [{}] executing job", std::hash<std::thread::id>{}(std::this_thread::get_id()) % 100);

		job->Execute();

		if (job->m_UnfinishedJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
			job->Finish(*this);
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
				OnDetach(job);
			}
			else
			{
				std::this_thread::yield();
			}

		}
	}

}