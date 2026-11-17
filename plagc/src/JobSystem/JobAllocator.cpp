#include "pch.h"
#include "JobAllocator.h"
#include "Utility/LockFreeIndexStack.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>

namespace plagc
{
    namespace
    {
        // Fixed at program startup. Change this to evaluate pool capacity.
        constexpr std::uint32_t kPoolCapacity = 16'384;

        struct Slot final
        {
            alignas(Job) std::byte storage[sizeof(Job)];
        };
        static_assert(std::is_standard_layout_v<Slot>);
        static_assert(offsetof(Slot, storage) == 0);

        class JobPool final
        {
        public:
            JobPool()
                : m_Free(kPoolCapacity),
                  m_Slots(std::make_unique<Slot[]>(kPoolCapacity))
            {
            }

            void* TryAcquire() noexcept
            {
                std::uint32_t index = 0;
                if (!m_Free.TryPop(index))
                    return nullptr;
                return static_cast<void*>(m_Slots[index].storage);
            }

            bool TryRelease(void* storage) noexcept
            {
                // Windows x64 and Linux x64: uintptr_t is a stable numerical
                // representation of object addresses.
                const auto ptr = reinterpret_cast<std::uintptr_t>(storage);
                const auto base = reinterpret_cast<std::uintptr_t>(m_Slots.get());
                if (ptr < base)
                    return false;

                const auto offset = ptr - base;
                const auto bytes = sizeof(Slot) * static_cast<std::size_t>(kPoolCapacity);
                if (offset >= bytes || offset % sizeof(Slot) != 0)
                    return false;

                const auto index = static_cast<std::uint32_t>(offset / sizeof(Slot));
                m_Free.Push(index);
                return true;
            }

        private:
            LockFreeIndexStack m_Free;
            std::unique_ptr<Slot[]> m_Slots;
        };

        JobPool& Pool()
        {
            // Thread-safe one-time initialization (C++11 and later).
            // Lifetime contract: all jobs/worker threads finish before static teardown.
            static JobPool pool;
            return pool;
        }
    }

    void* JobAllocator::AcquireStorage()
    {
        if (void* storage = Pool().TryAcquire())
            return storage;

        // Never drop a job if the fixed-size pool is exhausted.
        return ::operator new(sizeof(Job), std::align_val_t{alignof(Job)});
    }

    void JobAllocator::ReleaseStorage(void* storage) noexcept
    {
        if (!Pool().TryRelease(storage))
            ::operator delete(storage, std::align_val_t{alignof(Job)});
    }

    void JobAllocator::DeAllocate(Job* job) noexcept
    {
        if (!job)
            return;

        std::destroy_at(job); // destroys captures, FuncWrapper, and members
        ReleaseStorage(job);
    }
}
