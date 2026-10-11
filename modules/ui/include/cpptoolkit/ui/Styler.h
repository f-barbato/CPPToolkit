#pragma once

#include <cpptoolkit/ui/Export.h>
#include <imgui.h>

/** @file
 *  @brief Global ImGui theme and style configuration.
 */

namespace cpptoolkit::ui {

/** @brief Built-in ImGui color palettes. */
enum class Theme {
    Dark,    ///< Dark palette.
    Light,   ///< Light palette.
    Classic  ///< Classic palette.
};

/** @brief Own an ImGui style independently of an active context.
 *  @note Mutation and application belong to the render thread. SetTheme()
 *        replaces colors while preserving customized metrics.
 */
class CPPTOOLKIT_UI_EXPORT Styler {
public:
    /** @brief Create default metrics and the selected palette.
     *  @param theme Initial color palette.
     */
    explicit Styler(Theme theme = Theme::Dark);
    /** @brief Replace the color palette without resetting metrics.
     *  @param theme Color palette.
     *  @return This styler.
     *  @throws std::invalid_argument If theme is not a supported value.
     */
    Styler& SetTheme(Theme theme);
    /** @brief Query the selected palette.
     *  @return Palette used as the basis for custom colors.
     */
    [[nodiscard]] Theme GetTheme() const { return theme_; }
    /** @brief Access all colors and metrics for customization.
     *  @return Mutable style configuration; no context is required.
     */
    [[nodiscard]] ImGuiStyle& GetStyle() { return style_; }
    /** @brief Inspect configured colors and metrics.
     *  @return Owned style configuration.
     */
    [[nodiscard]] const ImGuiStyle& GetStyle() const { return style_; }
    /** @brief Apply the owned style to the current ImGui context.
     *  @note Call outside widget rendering and PushStyle/PopStyle scopes.
     *  @throws std::logic_error If there is no current ImGui context.
     */
    void Apply() const;

private:
    ImGuiStyle style_;
    Theme theme_ = Theme::Dark;
};

} // namespace cpptoolkit::ui
