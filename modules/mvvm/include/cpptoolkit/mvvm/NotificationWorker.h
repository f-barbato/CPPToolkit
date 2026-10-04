#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace cpptoolkit::mvvm {

// Single dedicated background thread executing posted tasks in FIFO order.
// Useful to run Observable<T>::Notify handlers off the caller's thread when
// handlers perform non-trivial work. Ordering is preserved, but all tasks
// serialize through this one worker (no parallel handler execution).
class NotificationWorker {
public:
    static NotificationWorker& Instance();

    void Post(std::function<void()> task);

    NotificationWorker(const NotificationWorker&) = delete;
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
