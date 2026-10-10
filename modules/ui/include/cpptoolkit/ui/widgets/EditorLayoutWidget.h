#pragma once

/** @file
 *  @brief Five-area dockable editor workspace.
 */

#include <string>
#include <cpptoolkit/ui/Panel.h>

namespace cpptoolkit::ui {

/** @brief Initial editor split arrangement; users can subsequently change docking. */
enum class EditorLayout {
    TopBottomFullWidth, ///< Top and bottom span the width; sidebars flank the center.
    LeftFullHeight, ///< Left spans the height; top and bottom start to its right.
    RightFullHeight, ///< Right spans the height; top and bottom end to its left.
    SidebarsFullHeight ///< Both sidebars span the height; top/bottom sit between them.
};

/** @brief Owned editor panel selected by the visibility API. */
enum class EditorRegion {
    Left, ///< Left sidebar.
    Right, ///< Right sidebar.
    Top, ///< Upper panel.
    Bottom, ///< Lower panel.
    Center ///< Central content.
};

/** @brief Viewport workspace with top, bottom, left, right and central panels.
 *
 * The selected preset controls split order. Each split reserves a fraction of
 * remaining space: top 15%, bottom 25%, left 20%, right 25%.
 * Users can resize, move, close or undock panels. Existing ImGui ini layouts
 * are preserved; ResetLayout() explicitly restores the initial splits.
 * Use one visible viewport workspace at a time and a stable unique identifier.
 * Panels inherit ambient MVVM/Application contexts and Render() traversal.
 * Populate the five accessors; inherited Add() is for additional windows.
 */
class CPPTOOLKIT_UI_EXPORT EditorLayoutWidget : public Widget {
public:
    /** @brief Create five owned panel windows without allocating graphics resources.
     *  @param identifier Stable, unique workspace ID used for docking and persistence.
     *  @param layout Initial arrangement used when no saved docking layout exists.
     *  @throws std::invalid_argument If identifier is empty or layout is invalid.
     */
    explicit EditorLayoutWidget(std::string identifier = "Editor",
                                EditorLayout layout = EditorLayout::TopBottomFullWidth);
    /** @brief Enable ImGui docking; requires a current ImGui context. */
    void PreBuild() override;
    /** @brief Submit the dockspace and draw panels when Visible.
     *  @note A hidden workspace keeps its docking nodes alive but does not draw children.
     */
    void Draw() override;
    /** @brief Restore the default arrangement and reopen all five panels on next visible Draw(). */
    void ResetLayout();
    /** @brief Select a preset and rebuild docking on the next visible Draw().
     *  @param layout New arrangement; reselecting the current preset does nothing.
     *  @note Preserves panel visibility. ResetLayout() explicitly reapplies a preset.
     *  @throws std::invalid_argument If layout is not a defined preset.
     */
    void SetLayout(EditorLayout layout);
    /** @brief Query the currently selected preset, not user-adjusted docking positions.
     *  @return Configured layout preset.
     */
    EditorLayout GetLayout() const { return layout_; }
    /** @brief Show/reopen or hide/close a panel without changing its RenderEnabled flag.
     *  @param region Panel to change.
     *  @param visible Whether to submit the panel window.
     *  @throws std::invalid_argument If region is not defined.
     */
    void SetPanelVisible(EditorRegion region, bool visible);
    /** @brief Query both the panel Visible flag and its open state, including user closing.
     *  @param region Panel to query.
     *  @return Whether the panel is enabled for display, independent of workspace Visible.
     *  @throws std::invalid_argument If region is not defined.
     */
    bool IsPanelVisible(EditorRegion region) const;
    /** @brief Access the left panel.
     *  @return Owned panel, valid until Destroy() or widget destruction.
     */
    Panel& Left() { return *left_; }
    /** @brief Access the right panel.
     *  @return Owned panel, valid until Destroy() or widget destruction.
     */
    Panel& Right() { return *right_; }
    /** @brief Access the top panel.
     *  @return Owned panel, valid until Destroy() or widget destruction.
     */
    Panel& Top() { return *top_; }
    /** @brief Access the bottom panel.
     *  @return Owned panel, valid until Destroy() or widget destruction.
     */
    Panel& Bottom() { return *bottom_; }
    /** @brief Access the central panel.
     *  @return Owned panel, valid until Destroy() or widget destruction.
     */
    Panel& Center() { return *center_; }

private:
    Panel& GetPanel(EditorRegion region) const;
    std::string identifier_;
    EditorLayout layout_;
    Panel* left_;
    Panel* right_;
    Panel* top_;
    Panel* bottom_;
    Panel* center_;
    bool reset_ = false;
    bool reopen_ = false;
};

} // namespace cpptoolkit::ui
