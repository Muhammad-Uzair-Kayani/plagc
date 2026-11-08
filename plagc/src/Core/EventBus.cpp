#include "pch.h"
#include "EventBus.h"

namespace plagc
{
	
	EventBus::EventBus(std::function<bool(Ref<Event>)> fn)
	{
		CORE_DEBUG("IN Event Bus");
		OnEvent = fn;
		if (!fn)
			throw std::logic_error("Cannot Assign Empty Callback");
	}

	void EventBus::PushEvent(Ref<Event> e)
	{
		m_Queue.push(e);
	}

	void EventBus::OnUpdate() noexcept
	{
		try
		{
			while (!m_Queue.empty())
			{
				OnEvent(m_Queue.front());
				m_Queue.pop();
			}
		}
		catch (std::bad_function_call e)
		{
			std::puts("Invalid Callback Call");
		}
		catch (std::exception e)
		{
			std::puts(e.what());
		}
	}

}