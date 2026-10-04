#include "cpptoolkit/mvvm/NotificationWorker.h"

namespace cpptoolkit::mvvm {

NotificationWorker& NotificationWorker::Instance() {
    static NotificationWorker instance;
    return instance;
}

NotificationWorker::NotificationWorker() : worker_([this] { Run(); }) {}

NotificationWorker::~NotificationWorker() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void NotificationWorker::Post(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(task));
    }
    cv_.notify_one();
}

void NotificationWorker::Run() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
            if (stop_ && queue_.empty()) return;
            task = std::move(queue_.front());
            queue_.pop();
        }
        task();
    }
}

} // namespace cpptoolkit::mvvm
