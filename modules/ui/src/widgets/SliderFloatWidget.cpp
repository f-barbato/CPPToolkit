#include "cpptoolkit/ui/widgets/SliderFloatWidget.h"

namespace cpptoolkit::ui{

    SliderFloatWidget::SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound, float min, float max)
        : label_(std::move(label)), bound_(bound), min_(min), max_(max) {}

    void SliderFloatWidget::Draw() {
        float value = bound_.Get();
        if (ImGui::SliderFloat(label_.c_str(), &value, min_, max_)) {
            bound_.Set(value);
        }
    }

} // namespace cpptoolkit::ui