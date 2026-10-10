#include "cpptoolkit/ui/widgets/SliderFloatWidget.h"

#include <cmath>
#include <stdexcept>

namespace cpptoolkit::ui{

    SliderFloatWidget::SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                                         float min, float max, std::function<void(float)> onValueChanged)
        : OnValueChanged(std::move(onValueChanged)), label_(std::move(label)), bound_(&bound),
          min_(min), max_(max) {
        if (!std::isfinite(min) || !std::isfinite(max) || min >= max)
            throw std::invalid_argument("SliderFloatWidget requires finite increasing endpoints");
    }

    void SliderFloatWidget::Draw() {
        float value = bound_ ? bound_->Get() : value_;
        const float previous = value;
        if (ImGui::SliderFloat(label_.c_str(), &value, min_, max_) && value != previous) {
            if (bound_) bound_->Set(value);
            else value_ = value;
            if (OnValueChanged) OnValueChanged(value);
        }
    }

    SliderFloatWidget::SliderFloatWidget(std::string label, float min, float max, float initial)
        : label_(std::move(label)), min_(min), max_(max), value_(initial) {
        if (!std::isfinite(min) || !std::isfinite(max) || min >= max)
            throw std::invalid_argument("SliderFloatWidget requires finite increasing endpoints");
    }

} // namespace cpptoolkit::ui