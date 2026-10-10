#pragma once

/** @file
 *  @brief Two-way observable float slider.
 */

#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Poll a borrowed float property and write back actual slider edits.
 *  @note Property updates precede OnValueChanged; programmatic updates are silent.
 */
class SliderFloatWidget : public Widget {
public:
    /** @brief Configure the slider and borrow its value.
     *  @param label ImGui label and ID.
     *  @param bound Property which must outlive this widget.
     *  @param min Lower slider endpoint.
     *  @param max Upper slider endpoint.
     *  @param onValueChanged Optional render-thread user-edit callback.
     *  @throws std::invalid_argument If endpoints are nonfinite or min is not less than max.
     */
    SliderFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound, float min, float max,
                      std::function<void(float)> onValueChanged = {});
    /** @brief Render and commit actual user edits. */
    void Draw() override;
    /** @brief Optional callback receiving the committed value on the render thread. */
    std::function<void(float)> OnValueChanged;

private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    float min_;
    float max_;
};

} // namespace cpptoolkit::ui
