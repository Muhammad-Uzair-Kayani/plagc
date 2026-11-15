#pragma once

#include "Core/Core.h"
#include "FunctionWrapper.h"

#include <thread>
#include <future>
#include <vector>


namespace plagc
{

	class Job
	{
	public:

		Job(FuncWrapper&& func) noexcept : m_Function(std::move(func))
		{

		}
		Job& operator=(FuncWrapper&& func) noexcept
		{
			m_Function = std::move(func);
			return *this;
		}

		Job(Job&& other) noexcept = default;
		Job& operator=(Job&& other) noexcept = default;

		Job(const FuncWrapper& wrapper) = delete;
		Job(const Job& wrapper) = delete;
		Job& operator=(const FuncWrapper& job) = delete;
		Job& operator=(const Job& job) = delete;

	private:

		FuncWrapper m_Function;
		std::vector<Job*> m_Continuations;
		Job* parent;
		uint32_t m_DependencyCount;
		uint32_t m_UnfinishedJobs;
	};

	class JobGraph
	{
	public:

		JobGraph() = default;



	private:


	};

	class JobSystem
	{
	public:

		template<typename F>
		

	private:

	};
}