#include "cpptoolkit/mvvm/Dispatcher.h"

namespace cpptoolkit::mvvm {

Dispatcher& Dispatcher::Main() {
    static Dispatcher instance;
    return instance;
}

void Dispatcher::Post(std::function<void()> action) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(std::move(action));
}

void Dispatcher::ProcessPending() {
    std::vector<std::function<void()>> toRun;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        toRun.swap(queue_);
    }
    for (auto& action : toRun) action();
}

} // namespace cpptoolkit::mvvm
