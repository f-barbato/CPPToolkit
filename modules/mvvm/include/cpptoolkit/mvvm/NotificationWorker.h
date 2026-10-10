#pragma once
#include <cpptoolkit/mvvm/Export.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

/**
 * @file NotificationWorker.h
 * @brief Serial background execution queue for notification-related tasks.
 */

namespace cpptoolkit::mvvm {

/**
 * @brief Executes posted tasks on one dedicated background thread in FIFO order.
 *
 * Instance() returns the process-wide worker. Tasks are serialized, not run in
 * parallel. During destruction, it processes tasks already queued before
 * exiting. Task exceptions are not caught; an uncaught exception on the worker
 * thread causes std::terminate.
 */
class CPPTOOLKIT_MVVM_EXPORT NotificationWorker {
public:
    /**
     * @brief Returns the process-wide notification worker.
     * @return Reference to the shared worker.
     */
    static NotificationWorker& Instance();

    /**
     * @brief Enqueues a task for the worker thread.
     * @param task Callable to execute after previously queued tasks.
     */
    void Post(std::function<void()> task);

    /** @brief Copy construction is disabled for the singleton worker. */
    NotificationWorker(const NotificationWorker&) = delete;
    /** @brief Copy assignment is disabled for the singleton worker. */
    NotificationWorker& operator=(const NotificationWorker&) = delete;

private:
    NotificationWorker();
    ~NotificationWorker();

    void Run();

    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> queue_;
    bool stop_ = false;
};

} // namespace cpptoolkit::mvvm
