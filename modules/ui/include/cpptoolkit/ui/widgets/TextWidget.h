#pragma once

#include <string>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Read-only text display. Binding is optional: pass an ObservableProperty
// to poll it every frame, or just a static string to display unbound text.
class TextWidget : public Widget {
public:
    explicit TextWidget(std::string text = "");
    explicit TextWidget(mvvm::ObservableProperty<std::string>& bound);
    void Draw() override;

private:
    mvvm::ObservableProperty<std::string>* bound_ = nullptr;
    std::string text_;
};

} // namespace cpptoolkit::ui
