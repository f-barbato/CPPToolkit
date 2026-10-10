#pragma once

#include <string>

#include "cpptoolkit/mvvm/Observable.h"

/**
 * @file ObservableObject.h
 * @brief Base class for objects that report named property changes.
 */

namespace cpptoolkit::mvvm {

/**
 * @brief Provides generic named property-change notifications for ViewModels.
 *
 * ObservableProperty can report changes through this object in addition to
 * its own per-property subscribers.
 */
class ObservableObject {
public:
    /** @brief Virtual destructor for derived observable objects. */
    virtual ~ObservableObject() = default;

    /**
     * @brief Registers for notifications from any named property.
     * @param handler Callback receiving the changed property name.
     * @return RAII subscription; destroying it unregisters the callback.
     */
    [[nodiscard]] Observable<std::string>::Subscription OnAnyPropertyChanged(
        Observable<std::string>::Handler handler) {
        return propertyChanged_.Subscribe(std::move(handler));
    }

    /**
     * @brief Notifies subscribers that a property changed.
     * @param propertyName Name of the changed or derived property.
     *
     * This may be called by ObservableProperty or directly for computed
     * properties. Notifications execute synchronously on the calling thread.
     */
    void NotifyPropertyChanged(const std::string& propertyName) { propertyChanged_.Notify(propertyName); }

private:
    Observable<std::string> propertyChanged_;
};

} // namespace cpptoolkit::mvvm
