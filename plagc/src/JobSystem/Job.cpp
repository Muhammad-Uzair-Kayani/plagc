#include "pch.h"
#include "Job.h"
#include "JobSystem.h"
#include "JobAllocator.h"

namespace plagc
{
	void Job::Finish(JobSystem& jobSystem)
	{
		// This is called only when own execution/cancellation AND all children
		// have finished. No other thread may access this Job afterward.
		const bool cancelled =
			m_State.load(std::memory_order_acquire) == JobState::Terminated ||
			jobSystem.IsStopping();

		for (Job* dependent : m_Continuations)
		{
			// Every prerequisite reports exactly once. If ANY prerequisite
			// was cancelled, the dependent must not run.
			if (cancelled)
				dependent->m_CancelRequested.store(true, std::memory_order_release);

			if (dependent->m_DependencyCount.fetch_sub(1,
				std::memory_order_acq_rel) == 1)
			{
				// Ready even if cancelled: OnDetach will perform cancellation
				// without running its callable, then propagate to dependents.
				jobSystem.PushJob(dependent);
			}
		}

		// Save pointers BEFORE the object might be reclaimed.
		Job* parent = m_Parent;
		if (parent &&
			parent->m_UnfinishedJobs.fetch_sub(1,
				std::memory_order_acq_rel) == 1)
		{
			parent->Finish(jobSystem);
		}

		m_Finished.store(true, std::memory_order_release);
		JobAllocator::DeAllocate(this); // Ends lifetime of *this*.
		jobSystem.OnJobDestroyed();     // Must be AFTER destruction.
	}
}
