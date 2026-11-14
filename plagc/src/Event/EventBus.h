#pragma once
#include "Core/Core.h"
#include "Core/log.h"
#include "Event/Event.h"
#include "Utility/MPSCQueue.h"


#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <utility>
#include <mutex>

namespace plagc
{

	struct Subscription
	{
	public:
		std::function<void(Event& e)> OnNotify;
		uint16_t ID;
		bool Active = true;
	};

	class EventBus;

	template<typename T>
	class SubscriptionHandler
	{
	public:
		SubscriptionHandler() noexcept = default;

		SubscriptionHandler(EventBus& bus, uint16_t id) noexcept
			: m_Bus(&bus), m_ID(id) {
		}

		~SubscriptionHandler()
		{
			Reset();
		}

		SubscriptionHandler(const SubscriptionHandler&) = delete;
		SubscriptionHandler& operator=(const SubscriptionHandler&) = delete;

		SubscriptionHandler(SubscriptionHandler&& other) noexcept
			: m_Bus(std::exchange(other.m_Bus, nullptr)),
			m_ID(std::exchange(other.m_ID, 0)) {
		}

		SubscriptionHandler& operator=(SubscriptionHandler&& other) noexcept
		{
			if (this != &other)
			{
				Reset();

				m_Bus = std::exchange(other.m_Bus, nullptr);
				m_ID = std::exchange(other.m_ID, 0);
			}

			return *this;
		}

		void Reset() noexcept;

	private:
		EventBus* m_Bus = nullptr;
		uint16_t m_ID = 0;
	};


	class EventBus
	{
	public:

		EventBus(size_t capacity) :  m_Queue(capacity) {}

		template<typename T>
		SubscriptionHandler<T> Subscribe(std::function<void(Event& e)> OnNotify);

		template<typename T>
		void UnSubscribe(uint16_t id);

		bool PushEvent(Owner<Event> e);

		void OnUpdate();


	private:

		void OnDetach(Owner<Event> e);

	private:

		MPSCQueue<Owner<Event>> m_Queue;
		std::unordered_map<EventType, std::vector<Subscription>> m_ObserversList;
		uint16_t m_NextID = 0;
		std::shared_mutex m_BucketMutex;
	};

	template<typename T>
	void SubscriptionHandler<T>::Reset() noexcept
	{
		if (m_Bus)
		{
			m_Bus->UnSubscribe<T>(m_ID);

			m_Bus = nullptr;
			m_ID = 0;
		}
	}

	template<typename T>
	void EventBus::UnSubscribe(uint16_t id)
	{
		std::shared_lock<std::shared_mutex> lock(m_BucketMutex);
		auto& observers = m_ObserversList.find(T::StaticType);

		if (observers == m_ObserversList.end())
			return;

		for (auto& observer : observers)
		{
			if (observer.ID == id)
			{
				observer.Active = false;
				CORE_DEBUG("Subscriber removed: {}", id);
			}
		}

	}

	template<typename T>
	SubscriptionHandler<T> EventBus::Subscribe(
		std::function<void(Event&)> OnNotify)
	{
		std::unique_lock<std::shared_mutex> lock(m_BucketMutex);

		uint16_t id = ++m_NextID;

		m_ObserversList[T::StaticType].push_back(
			Subscription{ id, std::move(OnNotify) }
		);

		CORE_DEBUG("Subscriber added: {}", id);

		return SubscriptionHandler<T>(*this, id);
	}

}