#include "pch.h"
#include "EventBus.h"

namespace plagc
{

	bool EventBus::PushEvent(Owner<Event>&& e)
	{
		if (m_Queue.EnQueue(std::move(e)))
			return true;

		if (e->GetCategory() == EventCategory::Critical)
		{
			CORE_DEBUG("Pushing Critical Event");
			return m_WaitBuffer.EnQueue(std::move(e));
		}

		return false;
		
	}

	void EventBus::OnUpdate()
	{
		std::unique_lock<std::mutex> lock(m_BucketMutex);

		Owner<Event> e;
		while (m_Queue.Try_Pop(e))
		{
			OnDetach(std::move(e));
		}
		while (m_WaitBuffer.Try_Pop(e))
		{
			OnDetach(std::move(e));
		}
	}

	void EventBus::OnDetach(Owner<Event> e)
	{
		auto Observers = m_ObserversList.find(e->GetType());

		if (Observers != m_ObserversList.end())
		{
			for (auto& Subscriber : Observers->second)
			{
				Subscriber.OnNotify(*e.get());
				if(e->GetStatus() == EventStatus::Consume) break;
			}

			return;
		}

		CORE_WARN("Event Type Not Added to Queue");
	}

}