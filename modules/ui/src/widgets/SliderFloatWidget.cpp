#include "cpptoolkit/ui/widgets/SliderFloatWidget.h"

#include <cmath>
#include <stdexcept>

namespace cpptoolkit::ui{

    SliderFloatWidget::SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                                         float min, float max, std::function<void(float)> onValueChanged)
        : OnValueChanged(std::move(onValueChanged)), label_(std::move(label)), bound_(bound),
          min_(min), max_(max) {
        if (!std::isfinite(min) || !std::isfinite(max) || min >= max)
            throw std::invalid_argument("SliderFloatWidget requires finite increasing endpoints");
    }

    void SliderFloatWidget::Draw() {
        float value = bound_.Get();
        const float previous = value;
        if (ImGui::SliderFloat(label_.c_str(), &value, min_, max_) && value != previous) {
            bound_.Set(value);
            if (OnValueChanged) OnValueChanged(value);
        }
    }

} // namespace cpptoolkit::ui