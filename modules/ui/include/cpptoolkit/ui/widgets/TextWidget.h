#pragma once

#include <string>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Read-only text display, polling a bound string property every frame.
class TextWidget : public Widget {
public:
    explicit TextWidget(mvvm::ObservableProperty<std::string>& bound);
    void Draw() override;

private:
    mvvm::ObservableProperty<std::string>& bound_;
};

} // namespace cpptoolkit::ui
