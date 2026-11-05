#pragma once
#include "Core/Event.h"
#include "log.h"

namespace plagc
{
	class EventBus
	{
	public:

		static void Init(std::function<bool(Ref<Event>)> fn)
		{
			CORE_DEBUG("IN Event Bus");
			OnEvent = fn;
		}

		static void PushEvent(Ref<Event> event)
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

		static std::queue<Ref<Event>> m_Queue;
		static std::function<bool(Ref<Event>)> OnEvent;

	};
}