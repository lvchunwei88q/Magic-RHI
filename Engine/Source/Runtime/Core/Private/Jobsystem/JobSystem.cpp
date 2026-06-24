/**
 * @file JobSystem.cpp
 * @brief Implementation of the engine-wide C++ worker thread pool.
 */

#include "CoreMinimal.h"
#include "Jobsystem/JobSystem.h"
#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Core
{
    AUTO_REGISTER_SINGLETON(JobSystem, High);

    namespace
    {
        std::mutex g_singletonMutex;

        uint32_t ResolveWorkerCount(uint32_t requested) noexcept
        {
            if (requested != 0) {
                return std::clamp<uint32_t>(requested, 1u, 32u);
            }
            const auto detected = std::thread::hardware_concurrency();
            if (detected <= 1) {
                // Single-core or hardware_concurrency() returned 0 (allowed by the
                // standard) → still spin up one worker so call sites have a thread
                // to dispatch to without having to special-case JobSystem absence.
                return 1u;
            }
            return std::clamp<uint32_t>(detected - 1, 1u, 32u);
        }

    } // namespace

    struct JobSystem::ThreadPool
    {
        std::vector<std::thread> workers;
        std::queue<Task> queue;
        std::mutex queueMutex;
        std::condition_variable queueCv;
        std::atomic<bool> running{ false };
    };

    bool JobSystem::Init()
    {
        std::lock_guard<std::mutex> guard(g_singletonMutex);

        m_pool = std::make_unique<ThreadPool>();
        m_pool->running.store(true, std::memory_order_release);

        const uint32_t resolved = ResolveWorkerCount(workerCount);
        m_pool->workers.reserve(resolved);
        for (uint32_t i = 0; i < resolved; ++i) {
            m_pool->workers.emplace_back([] { JobSystem::Get().WorkerLoop(); });
        }

        InfoCapture::Capture("JobSystem online with "+ std::to_string(resolved)+ " worker thread(s) (hw_concurrency="
        + std::to_string(std::thread::hardware_concurrency()) + ")");

        return true;
    }

    void JobSystem::Uninstall()
    {
        m_pool->running.store(false, std::memory_order_release);
        m_pool->queueCv.notify_all();

        for (auto& t : m_pool->workers) {
            if (t.joinable()) {
                t.join();
            }
        }
        m_pool->workers.clear();
    }

    JobHandle JobSystem::Schedule(JobFn job)
    {
        auto counter = std::make_shared<std::atomic<int>>(1);
        Task task{ std::move(job), counter };
        {
            std::lock_guard<std::mutex> guard(m_pool->queueMutex);
            m_pool->queue.push(std::move(task));
        }
        m_pool->queueCv.notify_one();
        return JobHandle(std::move(counter));
    }

    JobHandle JobSystem::ScheduleBatch(uint32_t count, std::function<JobFn(uint32_t)> factory)
    {
        if (count == 0) {
            return JobHandle();
        }

        auto counter = std::make_shared<std::atomic<int>>(static_cast<int>(count));
        {
            std::lock_guard<std::mutex> guard(m_pool->queueMutex);
            for (uint32_t i = 0; i < count; ++i) {
                m_pool->queue.push(Task{ factory(i), counter });
            }
        }
        m_pool->queueCv.notify_all();
        return JobHandle(std::move(counter));
    }

    void JobSystem::ParallelFor(uint32_t count, std::function<void(uint32_t)> body)
    {
        if (count == 0) {
            return;
        }
        auto handle = ScheduleBatch(count, [body](uint32_t i) -> JobFn { return [body, i] { body(i); }; });
        Wait(handle);
    }

    void JobSystem::Wait(const JobHandle& handle)
    {
        if (!handle.IsValid()) {
            return;
        }

        // Opportunistic helper-thread participation: instead of a passive
        // condition_variable wait, the joining thread runs queued tasks itself
        // until the counter zeroes out. This protects against priority inversion
        // when the main render thread joins on jobs scheduled by helpers.
        while (handle.m_counter->load(std::memory_order_acquire) > 0) {
            if (!TryRunOne()) {
                // Queue empty but our counter not yet zero → workers are still
                // executing the last task(s); just yield instead of busy-waiting.
                std::this_thread::yield();
            }
        }
    }

    bool JobSystem::TryRunOne()
    {
        Task task;
        {
            std::lock_guard<std::mutex> guard(m_pool->queueMutex);
            if (m_pool->queue.empty()) {
                return false;
            }
            task = std::move(m_pool->queue.front());
            m_pool->queue.pop();
        }

        if (task.fn) {
            task.fn();
        }
        if (task.counter) {
            task.counter->fetch_sub(1, std::memory_order_acq_rel);
        }
        return true;
    }

    void JobSystem::WorkerLoop()
    {
        while (m_pool->running.load(std::memory_order_acquire)) {
            Task task;
            {
                std::unique_lock<std::mutex> guard(m_pool->queueMutex);
                m_pool->queueCv.wait(guard, [this] { return !m_pool->queue.empty() || !m_pool->running.load(std::memory_order_acquire); });

                if (!m_pool->running.load(std::memory_order_acquire) && m_pool->queue.empty()) {
                    return;
                }

                if (m_pool->queue.empty()) {
                    continue;
                }

                task = std::move(m_pool->queue.front());
                m_pool->queue.pop();
            }

            if (task.fn) {
                // Errors in user-supplied jobs are bugs in the caller — let
                // them propagate so the worker thread terminates with a
                // diagnosable stack instead of silently dropping work.
                task.fn();
            }
            if (task.counter) {
                task.counter->fetch_sub(1, std::memory_order_acq_rel);
            }
        }
    }

    uint32_t JobSystem::GetWorkerCount() const noexcept
    {
        return static_cast<uint32_t>(m_pool->workers.size());
    }

} // namespace Core
