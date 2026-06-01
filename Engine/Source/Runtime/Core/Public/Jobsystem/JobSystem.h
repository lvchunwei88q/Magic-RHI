#pragma once
#include "Common/CORE_API.h"

#include "Subsystem/Subsystem.h"
#include "Subsystem/SubsystemTemplate.h" // AUTO_REGISTER_SINGLETON
#include "Tools/Singleton.h"
#include "Common/Compiler.h"
#include <memory>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace Core
{
    DISABLE_DLL_WARNINGS_PUSH;
    /**
     * @brief Opaque, move-only handle identifying a single scheduled job.
     *
     * Wait()-able through JobSystem. Multiple handles can share the same
     * underlying counter — useful for ParallelFor where we want to wait on
     * an entire batch.
     */
    class JobHandle
    {
    public:
        JobHandle() = default;
        explicit JobHandle(std::shared_ptr<std::atomic<int>> counter) : m_counter(std::move(counter))
        {
        }

        JobHandle(const JobHandle&) = default;
        JobHandle& operator=(const JobHandle&) = default;
        JobHandle(JobHandle&&) noexcept = default;
        JobHandle& operator=(JobHandle&&) noexcept = default;

        bool IsValid() const noexcept
        {
            return static_cast<bool>(m_counter);
        }

        /// @brief Non-blocking poll. Returns true once all referenced jobs have completed.
        bool IsComplete() const noexcept
        {
            return m_counter && m_counter->load(std::memory_order_acquire) == 0;
        }

    private:
        friend class JobSystem;
        std::shared_ptr<std::atomic<int>> m_counter;
    };

    /**
     * @brief Engine-wide worker thread pool singleton.
     *
     * Initialize() must be called once at engine startup; Shutdown() once at
     * engine teardown. Get() returns the singleton in between. The singleton
     * is intentionally NOT initialised lazily — silent worker startup during
     * arbitrary translation units' static init would race with the rest of
     * engine bring-up.
     */
    class CORE_API JobSystem : public Subsystem, public Singleton<JobSystem>
    {
    public:
        using JobFn = std::function<void()>;

        virtual bool Init();
        virtual void Uninstall();

        /// @brief Schedule a single job. Returns a handle for joining later.
        /// The handle stays valid even after the job completes — IsComplete()
        /// just observes a zeroed counter.
        JobHandle Schedule(JobFn job);

        /// @brief Schedule @p count jobs that share a single counter, useful
        /// for ParallelFor-style fan-out where the caller wants one wait point.
        /// @p factory is invoked once per index (0..count-1) on the calling
        /// thread to produce each individual job — keeps the hot scheduling
        /// loop free of allocation when the factory is a stateless lambda.
        JobHandle ScheduleBatch(uint32_t count, std::function<JobFn(uint32_t index)> factory);

        /// @brief Convenience: parallel-for over [0, count). Equivalent to
        /// ScheduleBatch + Wait but expresses intent more clearly at call sites.
        void ParallelFor(uint32_t count, std::function<void(uint32_t index)> body);

        /// @brief Block the caller until all jobs referenced by @p handle complete.
        /// Returns immediately when the handle is invalid or already complete.
        /// While blocked, the caller participates as an opportunistic worker —
        /// this prevents priority inversion when the main thread joins on jobs
        /// scheduled by helper code.
        void Wait(const JobHandle& handle);

        /// @brief How many worker threads are running. 0 if Shutdown.
        uint32_t GetWorkerCount() const noexcept;

    private:

        struct Task
        {
            JobFn fn;
            std::shared_ptr<std::atomic<int>> counter;
        };

        void WorkerLoop();
        bool TryRunOne(); // Returns true if a task was executed.

        uint8_t workerCount = 4u;
        // PIMPL 
        struct ThreadPool;
        std::unique_ptr<ThreadPool> m_pool;
    };

    AUTO_REGISTER_SINGLETON_INCLUDE(JobSystem);

    DISABLE_DLL_WARNINGS_POP;

} // namespace Core
