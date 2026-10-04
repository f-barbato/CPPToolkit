#pragma once

#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Two-way bound slider: reads the property on Draw(), writes back via Set()
// when the user drags the slider.
class SliderFloatWidget : public Widget {
public:
    SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound, float min, float max)
        : label_(std::move(label)), bound_(bound), min_(min), max_(max) {}

    void Draw() override {
        float value = bound_.Get();
        if (ImGui::SliderFloat(label_.c_str(), &value, min_, max_)) {
            bound_.Set(value);
        }
    }

private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    float min_;
    float max_;
};

} // namespace cpptoolkit::ui
