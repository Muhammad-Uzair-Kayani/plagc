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
		auto Observers = m_ObserversList[e->GetType()];
		for (auto& OnNotify : Observers)
			OnNotify(*e.get());
	}

}