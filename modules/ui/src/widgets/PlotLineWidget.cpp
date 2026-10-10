#include "cpptoolkit/ui/widgets/PlotLineWidget.h"

namespace cpptoolkit::ui{
    PlotLineWidget::PlotLineWidget(std::string label) : label_(std::move(label)) {}

    PlotLineWidget::PlotLineWidget(std::string label, mvvm::ObservableProperty<float>& bound)
        : label_(std::move(label)), bound_(&bound) {}

    void PlotLineWidget::Draw() {
        if (!bound_) {
            ImGui::TextUnformatted((label_ + ": no data source bound").c_str());
            return;
        }

        history_.Push(bound_->Get());

        scratch_.resize(history_.Size());
        for (std::size_t i = 0; i < history_.Size(); ++i) {
            scratch_[i] = history_.At(i);
        }

        if (ImPlot::BeginPlot(label_.c_str(), ImVec2(-1, 200))) {
            ImPlot::PlotLine("value", scratch_.data(), static_cast<int>(scratch_.size()));
            ImPlot::EndPlot();
        }
    }
} // namespace cpptoolkit::ui