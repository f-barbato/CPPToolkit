#pragma once

/** @file
 *  @brief Command-bound button.
 */

#include <functional>
#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/Command.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Button disabled when its borrowed Command cannot execute.
 *  @note The command must outlive the widget. OnClick executes after the command.
 */
class CPPTOOLKIT_UI_EXPORT ButtonWidget : public Widget {
public:
    
    /** @brief Bind a button action.
     *  @param label ImGui button label and ID.
     *  @param command Borrowed command.
     *  @param onClick Optional callback after a user activates the button.
     */
    ButtonWidget(std::string label, mvvm::Command& command, std::function<void()> onClick = {});
    /** @brief Render the button and invoke enabled user actions. */
    /** @brief Construct an optional callback-only button.
     *  @param label ImGui label and ID.
     *  @param onClick Optional callback; without it the button is inert.
     */
    explicit ButtonWidget(std::string label, std::function<void()> onClick = nullptr);
    /** @brief Render the button and invoke its enabled action. */
    void Draw() override;
    /** @brief Optional render-thread callback after command execution. */
    std::function<void()> OnClick;

private:
    std::string label_;
    mvvm::Command* command_ = nullptr;
};

} // namespace cpptoolkit::ui
