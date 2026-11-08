#include "pch.h"
#include "EventBus.h"

namespace plagc
{
	
	EventBus::EventBus(std::function<bool(Ref<Event>)> fn) noexcept
	{
		OnEvent = fn;
	}

	void EventBus::PushEvent(Ref<Event> e)
	{
		m_Queue.push(e);
	}

	void EventBus::OnUpdate()
	{
		while (!m_Queue.empty())
		{
			auto e = std::move(m_Queue.front());
			m_Queue.pop();
			OnEvent(e);
		}
	}

}