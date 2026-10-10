#pragma once

#include <mutex>
#include <string>
#include <utility>

#include "cpptoolkit/mvvm/Observable.h"
#include "cpptoolkit/mvvm/ObservableObject.h"

/**
 * @file ObservableProperty.h
 * @brief Mutex-protected observable property for ViewModels.
 */

namespace cpptoolkit::mvvm {

/**
 * @brief Holds a value and notifies listeners when it changes.
 *
 * Get() and the value comparison/update in Set() are protected by a mutex.
 * Set() invokes callbacks after releasing that mutex, synchronously on the
 * calling thread; listeners are not marshalled to another thread. Set() emits
 * no notifications when the new value compares equal to the current value.
 * Concurrent Set() calls are not serialized through their callback sequences.
 *
 * The owner pointer is non-owning and is used to notify the owner after the
 * per-property callbacks; a non-null owner must remain alive while Set() may
 * use it. Polling Get() from a render thread does not require Dispatcher.
 *
 * @tparam T Property value type, which must be equality-comparable and
 * copyable for the operations that use it.
 */
template <typename T>
class ObservableProperty {
public:
    /**
     * @brief Constructs a property with an optional owning observable object.
     * @param owner Non-owning object notified by name after a changed value.
     * @param name Property name reported to owner listeners.
     * @param initial Initial value, moved into the property.
     */
    explicit ObservableProperty(ObservableObject* owner, std::string name, T initial = T{})
        : owner_(owner), name_(std::move(name)), value_(std::move(initial)) {}

    /**
     * @brief Returns a copy of the current value while holding the property mutex.
     * @return Copy of the property's current value.
     */
    [[nodiscard]] T Get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

    /**
     * @brief Returns the property's name by const reference.
     * @return Reference to the name, valid while this property exists.
     */
    [[nodiscard]] const std::string& Name() const { return name_; }

    /**
     * @brief Replaces the value and notifies listeners if it compares unequal.
     * @param newValue Candidate value; it is copied into the property if changed.
     *
     * Per-property handlers receive the candidate value first. If they return
     * normally, the owner (when non-null) is then notified with Name().
     */
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

    /**
     * @brief Registers a listener for changes to this property's value.
     * @param handler Callback invoked with the new value after a successful
     * change.
     * @return RAII subscription; destroying it unregisters the handler.
     */
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
