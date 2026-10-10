#pragma once

/** @file
 *  @brief Read-only observable string display.
 */

#include <string>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Display a borrowed string property, polling it once per Draw(). */
class CPPTOOLKIT_UI_EXPORT TextWidget : public Widget {
public:
    /** @brief Bind read-only text.
     *  @param bound Property which must outlive this widget.
     */
    explicit TextWidget(mvvm::ObservableProperty<std::string>& bound);
    /** @brief Construct unbound static text.
     *  @param text Display value.
     */
    explicit TextWidget(std::string text = "");
    /** @brief Submit the current value as unformatted ImGui text. */
    void Draw() override;

private:
    mvvm::ObservableProperty<std::string>* bound_ = nullptr;
    std::string text_;
};

} // namespace cpptoolkit::ui
