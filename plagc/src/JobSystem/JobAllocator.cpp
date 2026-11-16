#include "pch.h"
#include "JobAllocator.h"

namespace plagc
{
	


	void JobAllocator::Dellocate(Job* job)
	{
		delete job;
	}

}