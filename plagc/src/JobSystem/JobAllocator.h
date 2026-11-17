#pragma once
#include "Job.h"

namespace plagc
{
	class JobAllocator
	{
	public:

		template<typename F, typename...Args>
		static Job* Allocate(F&& func, Args&&...args);

		static void DeAllocate(Job* job);

	private:


	};

	template<typename F, typename...Args>
	Job* plagc::JobAllocator::Allocate(F&& func, Args&&...args)
	{
		return new Job(std::forward<F>(func), std::forward<Args>(args)...);
	}

}