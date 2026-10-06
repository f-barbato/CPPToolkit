#pragma once

#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Slider, two-way bound when given an ObservableProperty (reads it on
// Draw(), writes back via Set() when dragged). Binding is optional: pass no
// property to drive a local, unbound value instead (e.g. plain tool options).
class SliderFloatWidget : public Widget {
public:
    SliderFloatWidget(std::string label, float min, float max, float initial = 0.0f);
    SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound, float min, float max);
    void Draw() override;

private:
    std::string label_;
    mvvm::ObservableProperty<float>* bound_ = nullptr;
    float min_;
    float max_;
    float value_;
};

} // namespace cpptoolkit::ui
