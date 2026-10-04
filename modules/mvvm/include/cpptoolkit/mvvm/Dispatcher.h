#pragma once

#include <functional>
#include <mutex>
#include <vector>

namespace cpptoolkit::mvvm {

// Marshals actions onto whichever thread calls ProcessPending() (typically
// the main/render thread). Only needed when a background-thread handler must
// call UI APIs directly (e.g. ImGui::OpenPopup); ordinary property binding
// via ObservableProperty<T> polling does not require this.
class Dispatcher {
public:
    static Dispatcher& Main();

    void Post(std::function<void()> action);

    // Call once per frame from the main/render thread.
    void ProcessPending();

private:
    std::mutex mutex_;
    std::vector<std::function<void()>> queue_;
};

} // namespace cpptoolkit::mvvm
