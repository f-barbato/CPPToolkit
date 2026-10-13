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

/** @brief Semantic color palette from which every ImGui widget color is derived.
 *  @note Alpha of each color is preserved only for Text and TextDisabled; other colors are used opaque.
 */
struct CPPTOOLKIT_UI_EXPORT Palette {
    ImVec4 Primary{0.26f, 0.59f, 0.98f, 1.0f};    ///< Interactive elements: buttons, check marks, selected tabs.
    ImVec4 Secondary{0.95f, 0.60f, 0.20f, 1.0f};  ///< Highlights: active sliders, plots, drag targets.
    ImVec4 Background{0.08f, 0.08f, 0.09f, 1.0f}; ///< Window and dockspace background.
    ImVec4 Surface{0.13f, 0.14f, 0.16f, 1.0f};    ///< Inputs, menus, popups, title bars and table headers.
    ImVec4 Text{0.93f, 0.93f, 0.94f, 1.0f};       ///< Primary text.
    ImVec4 TextDisabled{0.50f, 0.50f, 0.52f, 1.0f}; ///< Disabled text.
    ImVec4 Border{0.25f, 0.26f, 0.29f, 1.0f};     ///< Borders and separators.

    /** @brief Dark preset (default).
     *  @return Palette.
     */
    static Palette Dark() { return Palette{}; }
    /** @brief Light preset.
     *  @return Palette.
     */
    static Palette Light() {
        Palette p;
        p.Primary = {0.20f, 0.45f, 0.85f, 1.0f};
        p.Secondary = {0.85f, 0.40f, 0.10f, 1.0f};
        p.Background = {0.95f, 0.95f, 0.96f, 1.0f};
        p.Surface = {1.0f, 1.0f, 1.0f, 1.0f};
        p.Text = {0.08f, 0.08f, 0.10f, 1.0f};
        p.TextDisabled = {0.50f, 0.50f, 0.52f, 1.0f};
        p.Border = {0.78f, 0.79f, 0.82f, 1.0f};
        return p;
    }
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
    /** @brief Derive all ImGui colors from a semantic palette, preserving metrics.
     *  @param palette Primary/secondary/background palette.
     *  @return This styler.
     *  @note Individual colors may still be overridden afterwards through GetStyle().
     */
    Styler& SetPalette(const Palette& palette);
    /** @brief Query the last palette applied with SetPalette().
     *  @return Palette, or the dark preset if SetPalette() was never used.
     */
    [[nodiscard]] const Palette& GetPalette() const { return palette_; }
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
    Palette palette_;
    Theme theme_ = Theme::Dark;
};

} // namespace cpptoolkit::ui
