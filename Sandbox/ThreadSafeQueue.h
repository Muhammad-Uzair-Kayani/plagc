#pragma once
#include <deque>
#include <atomic>


template<typename T>
class ThreadSafeQueue
{
public:

	ThreadSafeQueue()
	{
		m_Queue.resize(256);
	}

	~ThreadSafeQueue() = default;

	void Push(T& val)
	{
		
	}

private:

	std::deque<T> m_Queue;
	std::atomic<int> m_Front;
	std::atomic<int> m_End;
};