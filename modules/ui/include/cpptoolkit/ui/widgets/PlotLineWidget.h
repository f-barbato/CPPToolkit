#pragma once

#include <string>
#include <utility>
#include <vector>

#include <implot.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>
#include <cpptoolkit/struct/RingBuffer.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Plots the recent history of a float property, sampling its current value
// once per Draw() call into a fixed-capacity cpptoolkit::structs::RingBuffer.
class PlotLineWidget : public Widget {
public:
    static constexpr std::size_t kHistoryCapacity = 256;

    PlotLineWidget(std::string label, mvvm::ObservableProperty<float>& bound)
        : label_(std::move(label)), bound_(bound) {}

    void Draw() override {
        history_.Push(bound_.Get());

        scratch_.resize(history_.Size());
        for (std::size_t i = 0; i < history_.Size(); ++i) {
            scratch_[i] = history_.At(i);
        }

        if (ImPlot::BeginPlot(label_.c_str(), ImVec2(-1, 200))) {
            ImPlot::PlotLine("value", scratch_.data(), static_cast<int>(scratch_.size()));
            ImPlot::EndPlot();
        }
    }

private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    structs::RingBuffer<float, kHistoryCapacity> history_;
    std::vector<float> scratch_;
};

} // namespace cpptoolkit::ui
