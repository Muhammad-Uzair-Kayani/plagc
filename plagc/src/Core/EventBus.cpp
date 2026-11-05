#include "pch.h"
#include "EventBus.h"

namespace plagc
{
	std::queue<Ref<Event>> EventBus::m_Queue;
	std::function<bool(Ref<Event>)> EventBus::OnEvent;

}