#pragma once

/** @file
 *  @brief Bound selectable items, sortable tables and hierarchical trees.
 */

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>
#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Toggle a borrowed selection flag through an ImGui selectable item. */
class CPPTOOLKIT_UI_EXPORT SelectableWidget : public Widget {
public:
    /** @brief Bind a selectable item.
     *  @param label Item label and ID.
     *  @param selected Borrowed flag which must outlive the widget.
     *  @param onSelectionChanged Optional callback after a user toggles the flag.
     */
    SelectableWidget(std::string label, mvvm::ObservableProperty<bool>& selected,
                     std::function<void(bool)> onSelectionChanged = {});
    /** @brief Poll and commit a user toggle before its callback. */
    void Draw() override;
    /** @brief Render-thread callback receiving the committed flag; programmatic updates are silent. */
    std::function<void(bool)> OnSelectionChanged;

private:
    std::string label_;
    mvvm::ObservableProperty<bool>& selected_;
};

/** @brief One ordered key in a table sort request. */
struct TableSortColumn {
    /** @brief Zero-based column index. */
    int Column;
    /** @brief True for ascending order, false for descending. */
    bool Ascending;
};

/** @brief Display bound string rows with row selection and optional sort requests.
 *
 * Properties must outlive the widget. Selection indices are zero-based;
 * -1 or an out-of-range value displays no selected row without changing the
 * property. Missing cells are blank and cells beyond the columns are not shown.
 * Sorting is delegated to the ViewModel; the widget never reorders source rows.
 */
class CPPTOOLKIT_UI_EXPORT TableWidget : public Widget {
public:
    /** @brief Row-major string cell data. */
    using Rows = std::vector<std::vector<std::string>>;
    /** @brief Bind a table.
     *  @param label ImGui table ID.
     *  @param columns Fixed display column names, between 1 and 64.
     *  @param rows Borrowed row data.
     *  @param selectedRow Borrowed selected row index.
     *  @param sortable Enable user sorting; handle requests via OnSortRequested.
     *  @throws std::invalid_argument If the column count is outside [1,64].
     */
    TableWidget(std::string label, std::vector<std::string> columns,
                mvvm::ObservableProperty<Rows>& rows,
                mvvm::ObservableProperty<int>& selectedRow, bool sortable = false);
    /** @brief Render clipped rows, commit selection and emit pending sort requests.
     *  @throws std::length_error If the row count exceeds the supported int index range.
     */
    void Draw() override;

    /** @brief User-selection callback after the selectedRow property is updated. */
    std::function<void(int)> OnSelectionChanged;
    /** @brief ViewModel sort request, including initial default sort and ordered multi-sort keys.
     *  @note Read/update Rows here to apply sorting. Empty keys mean no requested ordering.
     */
    std::function<void(const std::vector<TableSortColumn>&)> OnSortRequested;

private:
    std::string label_;
    std::vector<std::string> columns_;
    mvvm::ObservableProperty<Rows>& rows_;
    mvvm::ObservableProperty<int>& selectedRow_;
    bool sortable_;
};

/** @brief Immutable node data used to build a TreeViewWidget hierarchy. */
struct TreeNode {
    /** @brief Unique nonnegative node identifier. */
    int Id;
    /** @brief Display label. */
    std::string Label;
    /** @brief Owned descendants; an empty list makes this a leaf. */
    std::vector<TreeNode> Children;
};

/** @brief Render an immutable hierarchy with a borrowed selected node ID.
 *  @note -1 or an unknown ID displays no selection. Expansion is retained by ImGui.
 */
class CPPTOOLKIT_UI_EXPORT TreeViewWidget : public Widget {
public:
    /** @brief Construct a selectable tree.
     *  @param label Heading above the tree.
     *  @param nodes Fixed node hierarchy, copied/moved into the widget.
     *  @param selectedNode Borrowed ID property which must outlive the widget.
     *  @param onSelectionChanged Optional user-selection callback.
     *  @throws std::invalid_argument If IDs are negative or duplicated anywhere in the tree.
     */
    TreeViewWidget(std::string label, std::vector<TreeNode> nodes,
                   mvvm::ObservableProperty<int>& selectedNode,
                   std::function<void(int)> onSelectionChanged = {});
    /** @brief Render nodes and update the ID before a user-selection callback. */
    void Draw() override;
    /** @brief Render-thread callback after a new node is selected; polling is silent. */
    std::function<void(int)> OnSelectionChanged;

private:
    void DrawNodes(const std::vector<TreeNode>& nodes);
    std::string label_;
    std::vector<TreeNode> nodes_;
    mvvm::ObservableProperty<int>& selectedNode_;
};

} // namespace cpptoolkit::ui
