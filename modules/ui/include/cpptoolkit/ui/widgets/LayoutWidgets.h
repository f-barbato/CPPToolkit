#pragma once

/** @file
 *  @brief Retained stack, grid, scrolling and resizable pane layouts.
 */

#include <functional>
#include <string>

#include <imgui.h>
#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Panel.h"

namespace cpptoolkit::ui {

/** @brief Layout flow or splitter axis. */
enum class Orientation {
    Horizontal, ///< Left-to-right flow or side-by-side panes.
    Vertical    ///< Top-to-bottom flow or vertically stacked panes.
};

/** @brief Draw owned visible children in horizontal or vertical flow. */
class CPPTOOLKIT_UI_EXPORT StackPanel : public Panel {
public:
    /** @brief Set the stack direction.
     *  @param orientation Child flow direction.
     */
    explicit StackPanel(Orientation orientation = Orientation::Vertical);
    /** @brief Draw visible children in a scoped ImGui group. */
    void Draw() override;

private:
    Orientation orientation_;
};

/** @brief Place each visible direct child into the next cell of an ImGui table. */
class CPPTOOLKIT_UI_EXPORT GridPanel : public Panel {
public:
    /** @brief Configure a fixed-column grid.
     *  @param label ImGui table ID.
     *  @param columns Column count in [1,64].
     *  @throws std::invalid_argument If the column count is outside [1,64].
     */
    GridPanel(std::string label, int columns);
    /** @brief Draw visible children row by row. */
    void Draw() override;

private:
    std::string label_;
    int columns_;
};

/** @brief Border-framed child region with vertical and horizontal scrolling. */
class CPPTOOLKIT_UI_EXPORT ScrollPanel : public Panel {
public:
    /** @brief Configure the scroll region.
     *  @param label ImGui child-window ID.
     *  @param size Finite ImGui dimensions; zero fills the remaining region,
     *              negative values reserve space from the remaining region.
     *  @throws std::invalid_argument If either dimension is nonfinite.
     */
    ScrollPanel(std::string label, ImVec2 size = ImVec2(0, 200));
    /** @brief Draw children in the scrollable child window when it is visible. */
    void Draw() override;

private:
    std::string label_;
    ImVec2 size_;
};

/** @brief Two owned pane containers separated by a draggable divider.
 *
 * The bound first-pane size is borrowed and must outlive the widget. Valid
 * finite values are visually clamped to available space without writing back;
 * user drags update the property before OnSizeChanged. Invalid values or
 * insufficient space display a diagnostic rather than drawing invalid panes.
 * Populate First() and Second(); inherited direct Add() children are not panes.
 */
class CPPTOOLKIT_UI_EXPORT SplitterWidget : public Widget {
public:
    /** @brief Bind the first pane extent in pixels.
     *  @param label Divider's ImGui item ID.
     *  @param firstPaneSize Borrowed extent property.
     *  @param orientation Horizontal for side-by-side panes, vertical for stacked panes.
     *  @param minimumPaneSize Positive finite minimum extent for each pane.
     *  @throws std::invalid_argument If minimumPaneSize is nonpositive or nonfinite.
     */
    SplitterWidget(std::string label, mvvm::ObservableProperty<float>& firstPaneSize,
                   Orientation orientation = Orientation::Horizontal, float minimumPaneSize = 40);
    /** @brief Render both panes and commit actual divider drags. */
    void Draw() override;
    /** @brief Build both pane trees and any inherited direct children. */
    void Build() override;
    /** @brief Render both panes and direct children, regardless of display visibility. */
    void Render() override;
    /** @brief Destroy both pane trees and inherited direct children. */
    void Destroy() override;
    /** @brief Access the first pane container.
     *  @return Container owned for the lifetime of the splitter.
     */
    Panel& First();
    /** @brief Access the second pane container.
     *  @return Container owned for the lifetime of the splitter.
     */
    Panel& Second();
    /** @brief Render-thread callback after a drag commits the first-pane extent. */
    std::function<void(float)> OnSizeChanged;

private:
    std::string label_;
    mvvm::ObservableProperty<float>& firstPaneSize_;
    Orientation orientation_;
    float minimum_;
    Panel first_;
    Panel second_;
};

} // namespace cpptoolkit::ui
