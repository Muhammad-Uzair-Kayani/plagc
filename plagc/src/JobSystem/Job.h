#pragma once
#include "FunctionWrapper.h"

#include <vector>
#include <new>
#include <atomic>

namespace plagc
{
	class JobSystem;

	enum class JobState
	{
		Waiting,
		Ready,
		Running,
		Completed,
		Terminated
	};

	class Job
	{
	public:

		template<typename F>
		Job(F&& func, Job* parent = nullptr) noexcept : m_Function(std::forward<F>(func)), m_Parent(parent) 
		{
			m_UnfinishedJobs.fetch_add(1, std::memory_order_acq_rel);

			if (m_Parent)
				m_Parent->m_UnfinishedJobs.fetch_add(1, std::memory_order_acq_rel);
		}

		template<typename F>
		Job& operator=(F&& func)
		{
			m_Function = std::forward<F>(func);
			return *this;
		}

		Job(Job&& other) noexcept = default;
		Job& operator=(Job&& other) noexcept = default;

		Job(const FuncWrapper& wrapper) = delete;
		Job(const Job& wrapper) = delete;
		Job& operator=(const FuncWrapper& job) = delete;
		Job& operator=(const Job& job) = delete;

		void AddContinuation(Job* _job)
		{
			m_Continuations.push_back(_job);
		}

		const std::vector<Job*>& GetContinuations() const noexcept
		{
			return m_Continuations;
		}

		void Execute()
		{
			m_State = JobState::Running;
			m_Function();
			m_State = JobState::Completed;
		}

		void Finish(JobSystem& jobsystem);

	public:

		FuncWrapper m_Function;
		
		std::vector<Job*> m_Continuations;
		
		Job* m_Parent = nullptr;

		JobState m_State = JobState::Waiting;

		alignas(std::hardware_destructive_interference_size)std::atomic<uint32_t> m_DependencyCount{ 0 };
		alignas(std::hardware_destructive_interference_size)std::atomic<uint32_t> m_UnfinishedJobs{ 0 };
		alignas(std::hardware_destructive_interference_size)std::atomic<uint32_t> m_Finished{ false };
	};

}