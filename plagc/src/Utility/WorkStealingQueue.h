#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace plagc
{
    // Bounded Chase-Lev-style work-stealing deque for POINTERS.
    // Exactly ONE owner thread may call TryPush() and TryPop().
    // Any number of other threads may concurrently call TrySteal().
    // The deque does NOT own/delete the pointed-to objects.
    // IMPORTANT: Stop and join all users before destructing the deque.
    template<typename T>
    class WorkStealingQueue final
    {
        static_assert(std::is_pointer_v<T>, "WorkStealingQueue stores pointers (e.g. Job*)");

    public:
        explicit WorkStealingQueue(std::size_t capacity = 1024)
            : m_Capacity(ValidateCapacity(capacity)),
              m_Mask(m_Capacity - 1),
              m_Buffer(std::make_unique<std::atomic<T>[]>(m_Capacity))
        {
            for (std::size_t i = 0; i < m_Capacity; ++i)
                m_Buffer[i].store(nullptr, std::memory_order_relaxed);
        }

        WorkStealingQueue(const WorkStealingQueue&) = delete;
        WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;
        WorkStealingQueue(WorkStealingQueue&&) = delete;
        WorkStealingQueue& operator=(WorkStealingQueue&&) = delete;

        // OWNER ONLY. Returns false if full; caller retains job ownership.
        bool TryPush(T item) noexcept
        {
            assert(item != nullptr);
            const std::uint64_t bottom = m_Bottom.load(std::memory_order_relaxed);
            const std::uint64_t top = m_Top.load(std::memory_order_acquire);

            if (bottom - top >= m_Capacity)
                return false;

            m_Buffer[bottom & m_Mask].store(item, std::memory_order_relaxed);
            m_Bottom.store(bottom + 1, std::memory_order_release);
            return true;
        }

        // OWNER ONLY. Returns false when empty or when a thief won the last-item race.
        bool TryPop(T& out) noexcept
        {
            out = nullptr;
            const std::uint64_t oldBottom = m_Bottom.load(std::memory_order_relaxed);
            const std::uint64_t observedTop = m_Top.load(std::memory_order_acquire);
            if (oldBottom <= observedTop)
                return false;

            const std::uint64_t bottom = oldBottom - 1;
            m_Bottom.store(bottom, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_seq_cst);
            std::uint64_t top = m_Top.load(std::memory_order_relaxed);

            if (top > bottom)
            {
                m_Bottom.store(bottom + 1, std::memory_order_relaxed);
                return false;
            }

            T item = m_Buffer[bottom & m_Mask].load(std::memory_order_relaxed);
            if (top == bottom)
            {
                // Owner and thieves race for the final element.
                if (!m_Top.compare_exchange_strong(top, top + 1,
                        std::memory_order_seq_cst, std::memory_order_relaxed))
                    item = nullptr;
                m_Bottom.store(bottom + 1, std::memory_order_relaxed);
            }

            out = item;
            return item != nullptr;
        }

        // THIEVES (nonowners). One successful CAS claims exactly one item.
        bool TrySteal(T& out) noexcept
        {
            out = nullptr;
            std::uint64_t top = m_Top.load(std::memory_order_acquire);
            std::atomic_thread_fence(std::memory_order_seq_cst);
            const std::uint64_t bottom = m_Bottom.load(std::memory_order_acquire);
            if (top >= bottom)
                return false;

            T item = m_Buffer[top & m_Mask].load(std::memory_order_relaxed);
            if (!m_Top.compare_exchange_strong(top, top + 1,
                    std::memory_order_seq_cst, std::memory_order_relaxed))
                return false;

            out = item;
            return true;
        }

        std::size_t Capacity() const noexcept { return m_Capacity; }

    private:
        static std::size_t ValidateCapacity(std::size_t capacity)
        {
            if (capacity < 2 || (capacity & (capacity - 1)) != 0)
                throw std::invalid_argument("WorkStealingQueue capacity must be a power of two >= 2");
            return capacity;
        }

        const std::size_t m_Capacity;
        const std::size_t m_Mask;
        std::unique_ptr<std::atomic<T>[]> m_Buffer;

        alignas(64) std::atomic<std::uint64_t> m_Top{0};
        alignas(64) std::atomic<std::uint64_t> m_Bottom{0};
    };
}
