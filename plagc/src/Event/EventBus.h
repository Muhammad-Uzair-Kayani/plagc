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

		EventBus(size_t capacity) :  m_Queue(capacity), m_WaitBuffer(capacity / 4) {}

		template<typename T>
		SubscriptionHandler<T> Subscribe(std::function<void(Event& e)> OnNotify);

		template<typename T>
		void UnSubscribe(uint16_t id);

		bool PushEvent(Owner<Event>&& e);

		void OnUpdate();


	private:

		void OnDetach(Owner<Event> e);

	private:

		MPSCQueue<Owner<Event>> m_Queue;
		MPSCQueue<Owner<Event>> m_WaitBuffer;
		std::unordered_map<EventType, std::vector<Subscription>> m_ObserversList;
		uint16_t m_NextID = 0;
		std::mutex m_BucketMutex;
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
		std::unique_lock<std::mutex> lock(m_BucketMutex);
		auto observers = m_ObserversList.find(T::StaticType);

		if (observers == m_ObserversList.end())
			return;

		size_t count = std::erase_if(observers->second,
			[id](auto& observer)
			{
				return observer.ID == id;
			}
			);

		if(count > 0)
			CORE_DEBUG("Subscriber removed: {}", id);

	}

	template<typename T>
	SubscriptionHandler<T> EventBus::Subscribe(
		std::function<void(Event&)> OnNotify)
	{
		std::unique_lock<std::mutex> lock(m_BucketMutex);

		uint16_t id = ++m_NextID;

		m_ObserversList[T::StaticType].push_back(
			Subscription{ std::move(OnNotify), id }
		);

		CORE_DEBUG("Subscriber added: {}", id);

		return SubscriptionHandler<T>(*this, id);
	}

}