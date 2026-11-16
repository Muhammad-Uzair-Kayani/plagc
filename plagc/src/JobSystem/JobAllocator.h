#pragma once
#include "Job.h"

namespace plagc
{
	class JobAllocator
	{
	public:

		template<typename F>
		Job* Allocate(F&& func);

		void Dellocate(Job* job);

	private:


	};

	template<typename F>
	Job* plagc::JobAllocator::Allocate(F&& func)
	{
		return new Job(std::forward<F>(func));
	}

}