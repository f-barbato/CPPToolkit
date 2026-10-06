#include "cpptoolkit/ui/widgets/SliderFloatWidget.h"

namespace cpptoolkit::ui{

    SliderFloatWidget::SliderFloatWidget(std::string label, float min, float max, float initial)
        : label_(std::move(label)), min_(min), max_(max), value_(initial) {}

    SliderFloatWidget::SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound, float min, float max)
        : label_(std::move(label)), bound_(&bound), min_(min), max_(max), value_(0.0f) {}

    void SliderFloatWidget::Draw() {
        float value = bound_ ? bound_->Get() : value_;
        if (ImGui::SliderFloat(label_.c_str(), &value, min_, max_)) {
            if (bound_) bound_->Set(value);
            else value_ = value;
        }
    }

} // namespace cpptoolkit::ui