#pragma once

#include <string>

#include "cpptoolkit/mvvm/Observable.h"

namespace cpptoolkit::mvvm {

// Base class for ViewModels, mirroring CommunityToolkit.Mvvm's ObservableObject:
// provides a generic "a property changed" notification (by name), in addition
// to the per-property notifications that ObservableProperty<T> offers.
class ObservableObject {
public:
    virtual ~ObservableObject() = default;

    [[nodiscard]] Observable<std::string>::Subscription OnAnyPropertyChanged(
        Observable<std::string>::Handler handler) {
        return propertyChanged_.Subscribe(std::move(handler));
    }

    // Called by ObservableProperty<T> when its value changes; can also be
    // called manually for computed/derived properties.
    void NotifyPropertyChanged(const std::string& propertyName) { propertyChanged_.Notify(propertyName); }

private:
    Observable<std::string> propertyChanged_;
};

} // namespace cpptoolkit::mvvm
