#include "cpptoolkit/ui/widgets/LayoutWidgets.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace cpptoolkit::ui {

StackPanel::StackPanel(Orientation orientation) : orientation_(orientation) {}

void StackPanel::Draw() {
    if (!Visible) return;
    ImGui::PushID(this);
    ImGui::BeginGroup();
    bool first = true;
    ForEachVisibleChild([&](Widget& child) {
        if (!first && orientation_ == Orientation::Horizontal) ImGui::SameLine();
        child.Draw();
        first = false;
    });
    ImGui::EndGroup();
    ImGui::PopID();
}

GridPanel::GridPanel(std::string label, int columns) : label_(std::move(label)), columns_(columns) {
    if (columns < 1 || columns > 64)
        throw std::invalid_argument("GridPanel requires between 1 and 64 columns");
}

void GridPanel::Draw() {
    if (!Visible) return;
    ImGui::PushID(this);
    if (ImGui::BeginTable(label_.c_str(), columns_)) {
        ForEachVisibleChild([](Widget& child) {
            ImGui::TableNextColumn();
            child.Draw();
        });
        ImGui::EndTable();
    }
    ImGui::PopID();
}

ScrollPanel::ScrollPanel(std::string label, ImVec2 size)
    : label_(std::move(label)), size_(size) {
    if (!std::isfinite(size.x) || !std::isfinite(size.y))
        throw std::invalid_argument("ScrollPanel requires finite dimensions");
}

void ScrollPanel::Draw() {
    if (!Visible) return;
    ImGui::PushID(this);
    if (ImGui::BeginChild(label_.c_str(), size_, ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_HorizontalScrollbar))
        DrawChildren();
    ImGui::EndChild();
    ImGui::PopID();
}

SplitterWidget::SplitterWidget(std::string label, mvvm::ObservableProperty<float>& firstPaneSize,
                               Orientation orientation, float minimumPaneSize)
    : label_(std::move(label)), firstPaneSize_(firstPaneSize), orientation_(orientation),
      minimum_(minimumPaneSize) {
    if (!std::isfinite(minimum_) || minimum_ <= 0)
        throw std::invalid_argument("SplitterWidget requires a positive finite minimum pane size");
}

void SplitterWidget::Draw() {
    if (!Visible) return;
    constexpr float thickness = 6;
    const auto available = ImGui::GetContentRegionAvail();
    const bool horizontal = orientation_ == Orientation::Horizontal;
    const float total = horizontal ? available.x : available.y;
    const float space = total - thickness - ImGui::GetStyle().ItemSpacing[horizontal ? 0 : 1] * 2;
    if (space < minimum_ * 2) {
        ImGui::TextUnformatted("Not enough space for splitter panes");
        return;
    }
    const float current = firstPaneSize_.Get();
    if (!std::isfinite(current)) {
        ImGui::TextUnformatted("Invalid splitter size");
        return;
    }
    const float extent = std::clamp(current, minimum_, space - minimum_);
    ImGui::PushID(this);
    ImGui::BeginGroup();
    if (ImGui::BeginChild("first", horizontal ? ImVec2(extent, available.y) : ImVec2(available.x, extent),
                          ImGuiChildFlags_Borders))
        first_.Draw();
    ImGui::EndChild();
    if (horizontal) ImGui::SameLine();
    ImGui::InvisibleButton(label_.c_str(), horizontal ? ImVec2(thickness, available.y) :
                                                       ImVec2(available.x, thickness));
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
        ImGui::SetMouseCursor(horizontal ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
    if (ImGui::IsItemActive()) {
        const float delta = horizontal ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y;
        const float changed = std::clamp(extent + delta, minimum_, space - minimum_);
        if (delta != 0 && changed != current) {
            firstPaneSize_.Set(changed);
            if (OnSizeChanged) OnSizeChanged(changed);
        }
    }
    if (horizontal) ImGui::SameLine();
    if (ImGui::BeginChild("second", horizontal ? ImVec2(space - extent, available.y) :
                                               ImVec2(available.x, space - extent),
                          ImGuiChildFlags_Borders))
        second_.Draw();
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::PopID();
}

void SplitterWidget::Build() {
    first_.Build();
    second_.Build();
    BuildChildren();
}

void SplitterWidget::Destroy() {
    first_.Destroy();
    second_.Destroy();
    DestroyChildren();
}

Panel& SplitterWidget::First() { return first_; }
Panel& SplitterWidget::Second() { return second_; }

} // namespace cpptoolkit::ui
