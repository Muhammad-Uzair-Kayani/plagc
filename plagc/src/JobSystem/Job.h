#pragma once
#include "FunctionWrapper.h"

#include <vector>

namespace plagc
{
	class Job
	{
	public:

		template<typename F>
		Job(F&& func) noexcept : m_Function(std::forward<F>(func)) {}

		template<typename F>
		Job& operator=(F&& func) noexcept
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

		void Execute()
		{
			m_Function();
		}

	private:

		FuncWrapper m_Function;
		std::vector<Job*> m_Continuations;
		Job* parent = nullptr;
		uint32_t m_DependencyCount = 0;
		uint32_t m_UnfinishedJobs = 0;
	};

}