#pragma once
#include "Core/Event.h"
#include "log.h"

namespace plagc
{
	class EventBus
	{
	public:

		static void Init(std::function<bool(std::shared_ptr<Event>)> fn)
		{
			CORE_DEBUG("IN Event Bus");
			OnEvent = fn;
		}

		static void PushEvent(std::shared_ptr<Event> event)
		{
			m_Queue.push(event);
		}

		static void OnUpdate()
		{
			while (!m_Queue.empty())
			{

				auto event = m_Queue.front();
				m_Queue.pop();
				OnEvent(event);

			}
		}

	private:

		static std::queue<std::shared_ptr<Event>> m_Queue;
		static std::function<bool(std::shared_ptr<Event>)> OnEvent;

	};
}