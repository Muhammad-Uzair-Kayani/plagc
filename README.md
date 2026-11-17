# plagc — pooled JobAllocator (C++20)

Extract at the repository root. This replaces only:

- `plagc/src/JobSystem/JobAllocator.h`
- `plagc/src/JobSystem/JobAllocator.cpp`

And adds:

- `plagc/src/Utility/LockFreeIndexStack.h`

Existing `JobAllocator::Allocate(callable, parent)` and `JobAllocator::DeAllocate(job)` calls remain unchanged. The pool pre-allocates 16,384 aligned `Job` slots on first use, uses a tagged 64-bit lock-free free-index stack, and falls back to correctly aligned `::operator new` when all slots are in use. The `Job` constructor runs on each allocation; its destructor runs on each deallocation. No `Job` is retained alive in a free slot.

## Important integration requirements

- **Fix job lifetime first**: the GitHub commit inspected (`50f6391`) has `if (!m_Parent) return;` inside `Job::Finish()`, which leaks each root job. If your local shutdown implementation has already corrected this, no additional change is needed. All completed root jobs must eventually call `JobAllocator::DeAllocate(this)` exactly once, with no other thread retaining a pointer.
- The existing global `JobAllocator` API is preserved; **do not** `delete` pooled jobs manually. All jobs created by this allocator must be returned through `JobAllocator::DeAllocate`.
- The pool cannot repair concurrent use-after-free, double-deallocation, or a job graph that submits a job more than once.
- Existing `FuncWrapper` still makes a heap allocation for each callable. This change pools the `Job` object only, not callable storage.
- The fixed-capacity pool owns its slots until program exit. Shutdown must join workers and resolve live jobs before static destruction.
- This implementation targets 64-bit x86 Windows and Linux with lock-free `std::atomic<uint64_t>` and aligned allocation support.
- The free list uses a 32-bit generation tag; like many compact tagged pointers it is not mathematically immune to ABA after 2^32 successful stack updates. Use quiescent rebuild / wider tags if that horizon matters.
- Your existing `Job(F&&, Job*) noexcept` constructor may call operations that throw. If construction throws, that `noexcept` will terminate the program; the allocator cannot catch it. Consider correcting that separately.

## Benchmark

Raw allocator Release baseline (1,100,000 outer iterations × 5,000 dependent square roots):

- Parallel (10 workers): 5,727.1293 ms
- Sequential (single job): 48,691.9258 ms
- Speedup: 8.501977736×; 85.02% efficiency
- Both checksums: `2210552.032520394`

Rerun the exact workload with the pool and check equal checksums. The benchmark allocates only a few dozen jobs, so little difference is expected; for measuring allocator throughput, add a separate test with many tiny jobs.

`tests/` is not installed into the engine source tree; it documents validation methodology. This ZIP contains production files and this README only.
