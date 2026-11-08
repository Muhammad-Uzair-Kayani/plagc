#pragma once
#include "Core.h"
#include "Core/Event.h"

#include <functional>
#include <queue>

namespace plagc
{
	class EventBus
	{
	public:

		EventBus(std::function<bool(Ref<Event>)> fn) noexcept;

		void PushEvent(Ref<Event> e);

		void OnUpdate();

	private:

		std::queue<Ref<Event>> m_Queue;
		std::function<bool(Ref<Event>)> OnEvent;

	};
}