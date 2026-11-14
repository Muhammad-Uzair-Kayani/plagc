#pragma once
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#define CPU_PAUSE() _mm_pause()
#elif defined(__aarch64__) || defined(_M_ARM64)
#define CPU_PAUSE() asm volatile("yield" ::: "memory")
#else
#define CPU_PAUSE() std::this_thread::yield()
#endif

namespace plagc
{

	template<typename T>
	class SPMCQueue
	{
	public:

		SPMCQueue(size_t capacity) : m_Range(capacity)
		{
			if (capacity == 0)
				throw std::invalid_argument("Queue cannot be of Capacity 0");

			m_Queue.resize(m_Range);
		}

		bool Empty()
		{
			size_t rear = m_Rear.load(std::memory_order_acquire);
			size_t front = m_Front.load(std::memory_order_acquire);
			return static_cast<ptrdiff_t>(front - rear) >= 0
		}

		bool Try_Pop(T& e)
		{
			
			//Since we are dealing with multiple consumers than relaxed load is just a guess
			size_t front = m_Front.load(std::memory_order_relaxed);

			while (true)
			{

				//Producer may produce more data we must acquire changes
				size_t rear = m_Rear.load(std::memory_order_acquire);

				//Now we have to verify we are able to remove
				if (static_cast<ptrdiff_t>(front - rear) >= 0)
					return false;

				//Now synchronizing m_Front with other consumers
				if (m_Front.compare_exchange_weak(front, front + 1, std::memory_order_relaxed, std::memory_order_relaxed))
					break;

			}			

			//Now we are guaranteed safe to pop
			//Since queue is Single consumer no need to sync Consume operation as we have already synchronized with producers
			e = std::move(m_Queue[front % m_Range]);

			//Now we have to provide the message till what index this Consumer had consumed
			size_t expected = front;
			while (!m_Consumed.compare_exchange_weak(expected, expected + 1, std::memory_order_release, std::memory_order_relaxed))
			{
				expected = front;
				CPU_PAUSE();
			}

			return true;
		}

		bool EnQueue(T&& e)
		{
			//Single Producer no synchronization required for rear
			//Get the current m_Rear to enqueue to that position
			size_t rear = m_Rear.load(std::memory_order_relaxed);

			//We must also see the changes made by Consumers
			size_t front = m_Consumed.load(std::memory_order_acquire);
			
			// Since it is Ring Buffer So if size >= capacity then Drop the val means return
			if (static_cast<ptrdiff_t>(rear - front) >= static_cast<ptrdiff_t>(m_Range))
				return false;
			
			//Now we have to locate the right writing Index => rear % m_Range will provide the right index
			m_Queue[rear % m_Range] = std::move(e);
			
			//We must now release the changes
			m_Rear.store(rear + 1, std::memory_order_release);
			
			return true;
		}

	private:

		size_t m_Range;

		alignas(64) std::atomic<size_t> m_Front{ 0 };
		alignas(64) std::atomic<size_t> m_Rear{ 0 };
		alignas(64) std::atomic<size_t> m_Consumed{ 0 };

		std::vector<T> m_Queue;

	};
}