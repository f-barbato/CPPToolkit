#pragma once

#include <mutex>
#include <string>
#include <utility>

#include "cpptoolkit/mvvm/Observable.h"
#include "cpptoolkit/mvvm/ObservableObject.h"

namespace cpptoolkit::mvvm {

// Observable, mutex-protected property, analogous to C#'s [ObservableProperty].
//
// Thread-safety model: Get()/Set() are mutex-protected, so widgets can safely
// poll Get() every frame from the render thread while another thread calls
// Set() (e.g. a comm/telemetry thread) — no Dispatcher needed for this case.
// A Dispatcher is only required if an OnChanged handler must call UI APIs
// (e.g. ImGui::OpenPopup) directly from a background thread.
template <typename T>
class ObservableProperty {
public:
    explicit ObservableProperty(ObservableObject* owner, std::string name, T initial = T{})
        : owner_(owner), name_(std::move(name)), value_(std::move(initial)) {}

    [[nodiscard]] T Get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

    [[nodiscard]] const std::string& Name() const { return name_; }

    void Set(const T& newValue) {
        bool changed = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!(newValue == value_)) {
                value_ = newValue;
                changed = true;
            }
        }
        if (changed) {
            changed_.Notify(newValue);
            if (owner_) owner_->NotifyPropertyChanged(name_);
        }
    }

    [[nodiscard]] typename Observable<T>::Subscription OnChanged(typename Observable<T>::Handler handler) {
        return changed_.Subscribe(std::move(handler));
    }

private:
    ObservableObject* owner_;
    std::string name_;
    mutable std::mutex mutex_;
    T value_;
    Observable<T> changed_;
};

} // namespace cpptoolkit::mvvm
