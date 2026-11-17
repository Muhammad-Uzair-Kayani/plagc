#pragma once
#include "FunctionWrapper.h"

#include <atomic>
#include <cstdint>
#include <new>
#include <utility>
#include <vector>

namespace plagc
{
	class JobSystem;

	enum class JobState
	{
		Waiting, Ready, Running, Completed, Terminated
	};

	class Job
	{
	public:
		template<typename F>
		Job(F&& func, Job* parent = nullptr)
			: m_Function(std::forward<F>(func)), m_Parent(parent)
		{
			// 1 represents the job's own callable. Each child adds one.
			if (m_Parent)
				m_Parent->m_UnfinishedJobs.fetch_add(1, std::memory_order_acq_rel);
		}

		template<typename F>
		Job& operator=(F&& func)
		{
			m_Function = std::forward<F>(func);
			return *this;
		}

		Job(const Job&) = delete;
		Job& operator=(const Job&) = delete;
		Job(Job&&) = delete;             // Atomics and identity must not move.
		Job& operator=(Job&&) = delete;

		void AddContinuation(Job* job)
		{
			// Only while building a graph; never concurrently with execution.
			m_Continuations.push_back(job);
		}

		const std::vector<Job*>& GetContinuations() const noexcept
		{
			return m_Continuations;
		}

		void Execute()
		{
			m_State.store(JobState::Running, std::memory_order_relaxed);
			m_Function();
			m_State.store(JobState::Completed, std::memory_order_release);
		}

		// Called EXACTLY ONCE, when m_UnfinishedJobs goes from 1 to 0.
		// Deletes this Job. No caller may dereference it afterward.
		void Finish(JobSystem& jobSystem);

	public:
		FuncWrapper m_Function;
		std::vector<Job*> m_Continuations;
		Job* m_Parent = nullptr;
		std::atomic<JobState> m_State{ JobState::Waiting };
		std::atomic<bool> m_CancelRequested{ false };
		alignas(std::hardware_destructive_interference_size)
			std::atomic<std::uint32_t> m_DependencyCount{ 0 };
		alignas(std::hardware_destructive_interference_size)
			std::atomic<std::uint32_t> m_UnfinishedJobs{ 1 };
		// Never poll a Job* after the job might have been destroyed.
		std::atomic<bool> m_Finished{ false };
	};
}
