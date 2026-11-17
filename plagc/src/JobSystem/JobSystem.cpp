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
		m_ThreadPool.reserve(count);
		try
		{
			for (std::uint32_t i = 0; i < count; ++i)
				m_ThreadPool.emplace_back(&JobSystem::Try_Work, this);
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
		// Don't drop jobs when the queue is full. Help drain work instead.
		// The existing MPMCQueue accepts an rvalue reference to Job*.
		while (!m_JobQueue.EnQueue(std::move(job)))
		{
			Job* helping = nullptr;
			if (m_JobQueue.Try_Pop(helping))
				OnDetach(helping);
			else
				std::this_thread::yield();
		}
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

	void JobSystem::Try_Work()
	{
		for (;;)
		{
			Job* job = nullptr;
			if (m_JobQueue.Try_Pop(job))
			{
				OnDetach(job);
				continue;
			}

			if (IsStopping() && OutstandingJobs() == 0)
				return;

			std::this_thread::yield();
		}
	}
}
