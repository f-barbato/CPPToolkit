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

    PlotLineWidget(std::string label, mvvm::ObservableProperty<float>& bound);
    void Draw() override;

private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    structs::RingBuffer<float, kHistoryCapacity> history_;
    std::vector<float> scratch_;
};

} // namespace cpptoolkit::ui
