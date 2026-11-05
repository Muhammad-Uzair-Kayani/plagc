#include "pch.h"
#include "EventBus.h"

namespace plagc
{
	std::queue<std::shared_ptr<Event>> EventBus::m_Queue;
	std::function<bool(std::shared_ptr<Event>)> EventBus::OnEvent;

}