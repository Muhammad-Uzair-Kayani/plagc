#pragma once
#include "Core.h"
#include "log.h"
#include "Core/Event.h"

#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

namespace plagc
{
	class EventBus
	{
	public:

		EventBus() noexcept = default;

		template<typename T>
		void Subscribe(std::function<void(Event& e)> OnNotify);

		void PushEvent(Owner<Event> e);

		void OnUpdate();


	private:

		void OnDetach(Owner<Event> e);

	private:

		std::queue<Owner<Event>> m_Queue;
		std::unordered_map<EventType, std::vector<std::function<void(Event& e)>>> m_ObserversList;
	};


	template<typename T>
	void plagc::EventBus::Subscribe(std::function<void(Event& e)> OnNotify)
	{
		m_ObserversList[T::StaticType].push_back(OnNotify);
		CORE_DEBUG("Subscriber added");
	}

}