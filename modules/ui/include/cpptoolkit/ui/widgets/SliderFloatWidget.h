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
class CPPTOOLKIT_UI_EXPORT SliderFloatWidget : public Widget {
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
    /** @brief Construct an unbound slider using a local value.
     *  @param label ImGui label and ID.
     *  @param min Finite lower endpoint.
     *  @param max Finite upper endpoint, greater than min.
     *  @param initial Initial local value.
     *  @throws std::invalid_argument If the endpoints are invalid.
     */
    SliderFloatWidget(std::string label, float min, float max, float initial = 0.0f);
    /** @brief Render the slider and commit bound or local edits before the callback. */
    void Draw() override;
    /** @brief Optional callback receiving the committed value on the render thread. */
    std::function<void(float)> OnValueChanged;

private:
    std::string label_;
    mvvm::ObservableProperty<float>* bound_ = nullptr;
    float min_;
    float max_;
    float value_ = 0;
};

} // namespace cpptoolkit::ui
