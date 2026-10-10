#include "cpptoolkit/ui/widgets/DataWidgets.h"

#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace cpptoolkit::ui {

SelectableWidget::SelectableWidget(std::string label, mvvm::ObservableProperty<bool>& selected,
                                   std::function<void(bool)> onSelectionChanged)
    : OnSelectionChanged(std::move(onSelectionChanged)), label_(std::move(label)), selected_(selected) {}

void SelectableWidget::Draw() {
    bool selected = selected_.Get();
    if (ImGui::Selectable(label_.c_str(), &selected)) {
        selected_.Set(selected);
        if (OnSelectionChanged) OnSelectionChanged(selected);
    }
}

TableWidget::TableWidget(std::string label, std::vector<std::string> columns,
                         mvvm::ObservableProperty<Rows>& rows,
                         mvvm::ObservableProperty<int>& selectedRow, bool sortable)
    : label_(std::move(label)), columns_(std::move(columns)), rows_(rows),
      selectedRow_(selectedRow), sortable_(sortable) {
    if (columns_.empty() || columns_.size() > 64)
        throw std::invalid_argument("TableWidget requires between 1 and 64 columns");
}

void TableWidget::Draw() {
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Resizable;
    if (sortable_) flags |= ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
    if (!ImGui::BeginTable(label_.c_str(), static_cast<int>(columns_.size()), flags)) return;
    for (const auto& column : columns_) ImGui::TableSetupColumn(column.c_str());
    ImGui::TableHeadersRow();

    if (sortable_) {
        auto* specs = ImGui::TableGetSortSpecs();
        if (specs && specs->SpecsDirty && OnSortRequested) {
            std::vector<TableSortColumn> sort;
            for (int i = 0; i < specs->SpecsCount; ++i)
                sort.push_back({specs->Specs[i].ColumnIndex,
                                specs->Specs[i].SortDirection == ImGuiSortDirection_Ascending});
            OnSortRequested(sort);
            specs->SpecsDirty = false;
        }
    }

    const auto rows = rows_.Get();
    if (rows.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        ImGui::EndTable();
        throw std::length_error("TableWidget has too many rows");
    }
    const int selected = selectedRow_.Get();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rows.size()));
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            ImGui::TableNextRow();
            ImGui::PushID(row);
            for (std::size_t column = 0; column < columns_.size(); ++column) {
                ImGui::TableSetColumnIndex(static_cast<int>(column));
                const auto& cells = rows[static_cast<std::size_t>(row)];
                const char* value = column < cells.size() ? cells[column].c_str() : "";
                if (column == 0) {
                    // Separate the row ID from its displayed text (which may contain "##").
                    const ImVec2 textPosition = ImGui::GetCursorScreenPos();
                    if (ImGui::Selectable("##row", selected == row, ImGuiSelectableFlags_SpanAllColumns)) {
                        if (selected != row) {
                            selectedRow_.Set(row);
                            if (OnSelectionChanged) OnSelectionChanged(row);
                        }
                    }
                    ImGui::GetWindowDrawList()->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), value);
                } else {
                    ImGui::TextUnformatted(value);
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
}

namespace {

void ValidateNodes(const std::vector<TreeNode>& nodes, std::unordered_set<int>& ids) {
    for (const auto& node : nodes) {
        if (node.Id < 0 || !ids.insert(node.Id).second)
            throw std::invalid_argument("TreeViewWidget node IDs must be unique and nonnegative");
        ValidateNodes(node.Children, ids);
    }
}

} // namespace

TreeViewWidget::TreeViewWidget(std::string label, std::vector<TreeNode> nodes,
                               mvvm::ObservableProperty<int>& selectedNode,
                               std::function<void(int)> onSelectionChanged)
    : OnSelectionChanged(std::move(onSelectionChanged)), label_(std::move(label)),
      nodes_(std::move(nodes)), selectedNode_(selectedNode) {
    std::unordered_set<int> ids;
    ValidateNodes(nodes_, ids);
}

void TreeViewWidget::Draw() {
    ImGui::PushID(this);
    ImGui::TextUnformatted(label_.c_str());
    DrawNodes(nodes_);
    ImGui::PopID();
}

void TreeViewWidget::DrawNodes(const std::vector<TreeNode>& nodes) {
    for (const auto& node : nodes) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (node.Children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (selectedNode_.Get() == node.Id) flags |= ImGuiTreeNodeFlags_Selected;
        ImGui::PushID(node.Id);
        const bool open = ImGui::TreeNodeEx("##node", flags, "%s", node.Label.c_str());
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && selectedNode_.Get() != node.Id) {
            selectedNode_.Set(node.Id);
            if (OnSelectionChanged) OnSelectionChanged(node.Id);
        }
        if (open && !node.Children.empty()) {
            DrawNodes(node.Children);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

} // namespace cpptoolkit::ui
