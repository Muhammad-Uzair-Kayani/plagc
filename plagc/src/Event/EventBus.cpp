#include "pch.h"
#include "EventBus.h"

namespace plagc
{

	void EventBus::PushEvent(Owner<Event> e)
	{
		m_Queue.push(std::move(e));
	}

	void EventBus::OnUpdate()
	{
		while (!m_Queue.empty())
		{
			auto e = std::move(m_Queue.front());
			m_Queue.pop();
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