#include "pch.h"
#include "JobAllocator.h"

namespace plagc
{
	


	void JobAllocator::DeAllocate(Job* job)
	{
		delete job;
	}

}