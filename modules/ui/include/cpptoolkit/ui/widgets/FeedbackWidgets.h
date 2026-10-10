#pragma once

/** @file
 *  @brief Observable progress, activity, status, notifications and image display.
 */

#include <functional>
#include <string>

#include <imgui.h>
#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Display a borrowed normalized progress value without modifying it.
 *  @note Finite values are visually clamped to [0,1]; nonfinite values display an error.
 */
class CPPTOOLKIT_UI_EXPORT ProgressBarWidget : public Widget {
public:
    /** @brief Bind a progress display.
     *  @param label Overlay text displayed on the bar.
     *  @param progress Borrowed property which must outlive this widget.
     *  @param size ImGui bar size; negative width fills the available region.
     */
    ProgressBarWidget(std::string label, mvvm::ObservableProperty<float>& progress,
                      ImVec2 size = ImVec2(-1, 0));
    /** @brief Poll the property and submit the progress bar or invalid-value message. */
    void Draw() override;

private:
    std::string label_;
    mvvm::ObservableProperty<float>& progress_;
    ImVec2 size_;
};

/** @brief Render an animated activity arc only while a borrowed busy flag is true. */
class CPPTOOLKIT_UI_EXPORT SpinnerWidget : public Widget {
public:
    /** @brief Bind an activity indicator.
     *  @param label Text accompanying the arc.
     *  @param busy Borrowed busy flag which must outlive this widget.
     */
    SpinnerWidget(std::string label, mvvm::ObservableProperty<bool>& busy);
    /** @brief Draw an ImGui-time-based animation without altering the busy flag. */
    void Draw() override;

private:
    std::string label_;
    mvvm::ObservableProperty<bool>& busy_;
};

/** @brief Display a borrowed status string in a fixed color. */
class CPPTOOLKIT_UI_EXPORT BadgeWidget : public Widget {
public:
    /** @brief Bind colored status text.
     *  @param text Borrowed property which must outlive this widget.
     *  @param color Text RGBA color.
     */
    explicit BadgeWidget(mvvm::ObservableProperty<std::string>& text,
                         ImVec4 color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    /** @brief Poll and render colored status text. */
    void Draw() override;

private:
    mvvm::ObservableProperty<std::string>& text_;
    ImVec4 color_;
};

/** @brief Inline notification with a user-dismiss button and bound visibility.
 *  @note Both properties must outlive this widget; programmatic closing is silent.
 */
class CPPTOOLKIT_UI_EXPORT NotificationWidget : public Widget {
public:
    /** @brief Bind notification content and visibility.
     *  @param label Prefix text.
     *  @param message Borrowed notification text.
     *  @param open Borrowed visibility flag.
     */
    NotificationWidget(std::string label, mvvm::ObservableProperty<std::string>& message,
                       mvvm::ObservableProperty<bool>& open);
    /** @brief Draw while open and commit a user dismissal before invoking its callback. */
    void Draw() override;

    /** @brief Render-thread callback after the user sets the open property to false. */
    std::function<void()> OnDismissed;

private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& message_;
    mvvm::ObservableProperty<bool>& open_;
};

/** @brief Display a borrowed backend texture identifier without owning its texture.
 *  @note The property and backend texture must remain alive through rendering.
 */
class CPPTOOLKIT_UI_EXPORT ImageWidget : public Widget {
public:
    /** @brief Configure an image.
     *  @param texture Borrowed texture identifier property.
     *  @param size Positive, finite display dimensions in pixels.
     *  @throws std::invalid_argument If either dimension is nonpositive or nonfinite.
     */
    ImageWidget(mvvm::ObservableProperty<ImTextureID>& texture, ImVec2 size);
    /** @brief Poll the texture identifier and submit an ImGui image. */
    void Draw() override;

private:
    mvvm::ObservableProperty<ImTextureID>& texture_;
    ImVec2 size_;
};

} // namespace cpptoolkit::ui
