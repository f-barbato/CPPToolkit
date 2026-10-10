#pragma once

/** @file
 *  @brief Rolling-history ImPlot line plot.
 */

#include <string>
#include <utility>
#include <vector>

#include <implot.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>
#include <cpptoolkit/struct/RingBuffer.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Sample a borrowed float property on every Draw() into a rolling history.
 *  @note Samples are frame-based, not timestamped. The property must outlive the widget.
 */
class PlotLineWidget : public Widget {
public:
    /** @brief Maximum number of retained samples. */
    static constexpr std::size_t kHistoryCapacity = 256;

    /** @brief Configure a rolling plot.
     *  @param label ImPlot plot title and ID.
     *  @param bound Borrowed float source.
     */
    PlotLineWidget(std::string label, mvvm::ObservableProperty<float>& bound);
    /** @brief Append a sample and submit the retained history to ImPlot. */
    void Draw() override;

private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    structs::RingBuffer<float, kHistoryCapacity> history_;
    std::vector<float> scratch_;
};

} // namespace cpptoolkit::ui
