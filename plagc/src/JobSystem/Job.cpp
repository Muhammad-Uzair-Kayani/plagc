#include "pch.h"
#include "Job.h"
#include "JobSystem.h"
#include "JobAllocator.h"

namespace plagc
{

	void Job::Finish(JobSystem& jobSystem)
	{
	
		if (this->m_State == JobState::Terminated)
		{
			for (auto dependentJob : m_Continuations)
				if (dependentJob->m_DependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				{
					dependentJob->m_State = JobState::Terminated;
					dependentJob->Finish(jobSystem);
				}
		}

		for (auto dependentJob : m_Continuations)
			if (dependentJob->m_DependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				jobSystem.PushJob(dependentJob);

		if (!m_Parent)
			return;

		if(m_Parent->m_UnfinishedJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
				m_Parent->Finish(jobSystem);

		JobAllocator::DeAllocate(this);
	}

}