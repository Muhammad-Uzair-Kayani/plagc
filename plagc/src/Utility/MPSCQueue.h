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
	class MPSCQueue
	{
	public:

		MPSCQueue(size_t capacity) : m_Range(capacity)
		{
			if (capacity == 0)
				throw std::invalid_argument("Queue cannot be of Capacity 0");

			m_Queue.resize(m_Range);
		}

		bool Try_Pop(T& e)
		{
			//To Pop we have to get the front in same way
			//Relax is allowed since we are dealing with single Consumer, This front is guaranteed to be true
			size_t front = m_Front.load(std::memory_order_relaxed);

			//We have to acquire the changes and m_Committed index to check invariant
			size_t rear = m_Produced.load(std::memory_order_acquire);

			//Now checking if we are within the popping range
			if (front >= rear)
				return false;

			//Now we are guaranteed safe to pop
			//Since queue is Single consumer no need to sync Consume operation as we have already synchronized with producers
			e = std::move(m_Queue[front % m_Range]);

			//Now after popping we atomically increase m_Front by releasing the changes we have made!
			m_Front.store(front + 1, std::memory_order_release);

			return true;
		}

		bool EnQueue(T e)
		{
			//Multiple Producers could enqueue so we have to synchronize the insertion
			//Get the current m_Front to enqueue to that position
			size_t rear = m_Rear.load(std::memory_order_relaxed); //Relaxed load is allowed since we will synchronize the operation
			// Since it is Ring Buffer So if size >= capacity then Drop the Event means return
			while (true)
			{
				size_t front = m_Front.load(std::memory_order_acquire);

				if ( static_cast<ptrdiff_t>(rear - front) >= static_cast<ptrdiff_t>(m_Range) )
					return false;

				if (m_Rear.compare_exchange_weak(rear, rear + 1, std::memory_order_relaxed, std::memory_order_relaxed))
					break;
			}

			//Now we have to locate the right writing Index => rear % m_Range will provide the right index
			m_Queue[rear % m_Range] = std::move(e);

			//Now we have assigned the val we have to commit the assignment
			size_t expected = rear;
			//Now if we have right val for committing index than we are safe to exit else we have to wait for other
			while (!m_Produced.compare_exchange_weak(expected, rear + 1, std::memory_order_release, std::memory_order_relaxed))
			{
				//If we fail than we must reset the expected index to what we want not the one that currently is:
				expected = rear;
				//We will shortly make this thread sleep to not to waste resource
				CPU_PAUSE();
			}
			//Now the m_Committed will always provide range of valid values
			return true;
		}

	private:

		size_t m_Range;

		alignas(64) std::atomic<size_t> m_Front{0};
		alignas(64) std::atomic<size_t> m_Rear{0};
		alignas(64) std::atomic<size_t> m_Produced{0};

		std::vector<T> m_Queue;

	};
}