#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>

namespace plagc
{
    // A fixed-size MPMC free-index stack. No malloc/free on its push/pop path.
    // A 32-bit generation counter in the 64-bit head prevents ordinary ABA
    // when a node is popped, returned, and pushed again.
    // Requirement: indices must not be pushed twice without an intervening pop.
    class LockFreeIndexStack final
    {
    public:
        static constexpr std::uint32_t Invalid =
            (std::numeric_limits<std::uint32_t>::max)();

        explicit LockFreeIndexStack(std::uint32_t capacity)
            : m_Next(capacity ? std::make_unique<std::atomic<std::uint32_t>[]>(capacity)
                              : nullptr)
        {
            static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                "LockFreeIndexStack requires lock-free 64-bit atomics");

            for (std::uint32_t i = 0; i < capacity; ++i)
                m_Next[i].store(i + 1u == capacity ? Invalid : i + 1u,
                                std::memory_order_relaxed);

            m_Head.store(Pack(0, capacity ? 0u : Invalid),
                         std::memory_order_relaxed);
        }

        LockFreeIndexStack(const LockFreeIndexStack&) = delete;
        LockFreeIndexStack& operator=(const LockFreeIndexStack&) = delete;

        bool TryPop(std::uint32_t& index) noexcept
        {
            auto old = m_Head.load(std::memory_order_acquire);
            for (;;)
            {
                const auto headIndex = Index(old);
                if (headIndex == Invalid)
                    return false;

                // Atomic because a losing popper can still read this field
                // while a winning popper recycles the same index.
                const auto next = m_Next[headIndex].load(std::memory_order_relaxed);
                const auto desired = Pack(Generation(old) + 1u, next);

                if (m_Head.compare_exchange_weak(
                        old, desired,
                        std::memory_order_acquire,
                        std::memory_order_acquire))
                {
                    index = headIndex;
                    return true;
                }
            }
        }

        void Push(std::uint32_t index) noexcept
        {
            auto old = m_Head.load(std::memory_order_relaxed);
            for (;;)
            {
                m_Next[index].store(Index(old), std::memory_order_relaxed);
                const auto desired = Pack(Generation(old) + 1u, index);

                if (m_Head.compare_exchange_weak(
                        old, desired,
                        std::memory_order_release,
                        std::memory_order_relaxed))
                    return;
            }
        }

    private:
        static constexpr std::uint64_t Pack(
            std::uint32_t generation, std::uint32_t index) noexcept
        {
            return (static_cast<std::uint64_t>(generation) << 32) | index;
        }

        static constexpr std::uint32_t Index(std::uint64_t value) noexcept
        {
            return static_cast<std::uint32_t>(value);
        }

        static constexpr std::uint32_t Generation(std::uint64_t value) noexcept
        {
            return static_cast<std::uint32_t>(value >> 32);
        }

        std::unique_ptr<std::atomic<std::uint32_t>[]> m_Next;
        alignas(64) std::atomic<std::uint64_t> m_Head{Pack(0, Invalid)};
    };
}
