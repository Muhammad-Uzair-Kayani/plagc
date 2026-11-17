#pragma once

#include "Core/Core.h"
#include "Job.h"
#include <memory>
#include <new>
#include <utility>

namespace plagc
{
    // Fixed-capacity, thread-safe object pool with aligned heap fallback.
    // Existing Allocate/DeAllocate call sites remain unchanged.
    class JobAllocator final
    {
    public:
        template<typename F, typename... Args>
        static Job* Allocate(F&& func, Args&&... args)
        {
            void* storage = AcquireStorage();
            try
            {
                return std::construct_at(static_cast<Job*>(storage),
                    std::forward<F>(func), std::forward<Args>(args)...);
            }
            catch (...)
            {
                ReleaseStorage(storage);
                throw;
            }
        }

        static void DeAllocate(Job* job) noexcept;

    private:
        static void* AcquireStorage();
        static void ReleaseStorage(void* storage) noexcept;
    };
}
