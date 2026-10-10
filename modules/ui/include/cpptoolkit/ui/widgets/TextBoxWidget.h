#pragma once

/** @file
 *  @brief Single-line text input with two-way MVVM binding.
 */

#include <functional>
#include <string>
#include <vector>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Edit a borrowed UTF-8 string property with an automatically growing buffer.
 *
 * The property must outlive this widget. User edits commit the property before
 * OnTextChanged executes on the render thread. Programmatic changes are polled
 * without triggering this callback.
 */
class CPPTOOLKIT_UI_EXPORT TextBoxWidget : public Widget {
public:
    /** @brief Callback receiving the edited text after property commit. */
    using TextChangedHandler = std::function<void(const std::string&)>;

    /** @brief Construct the text binding.
     *  @param label ImGui label and item ID.
     *  @param bound Borrowed text property.
     *  @param onTextChanged Optional user-edit handler.
     */
    TextBoxWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                  TextChangedHandler onTextChanged = {});
    /** @brief Poll the text and submit an ImGui single-line input. */
    void Draw() override;

public:
    /** @brief Optional render-thread handler invoked after actual user edits. */
    TextChangedHandler OnTextChanged;
    /** @brief Disable editing while continuing to poll the bound value. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& bound_;

    std::vector<char> buffer_;
};

} // namespace cpptoolkit::ui
