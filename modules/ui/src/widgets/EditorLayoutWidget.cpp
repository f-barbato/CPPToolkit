#include <cpptoolkit/ui/widgets/EditorLayoutWidget.h>

#include <stdexcept>
#include <utility>
#include <array>
#include <imgui_internal.h>

namespace cpptoolkit::ui {
namespace {

std::array<EditorRegion, 4> SplitOrder(EditorLayout layout) {
    using enum EditorRegion;
    switch (layout) {
    case EditorLayout::TopBottomFullWidth: return {Top, Bottom, Left, Right};
    case EditorLayout::LeftFullHeight: return {Left, Top, Bottom, Right};
    case EditorLayout::RightFullHeight: return {Right, Top, Bottom, Left};
    case EditorLayout::SidebarsFullHeight: return {Left, Right, Top, Bottom};
    }
    throw std::invalid_argument("Invalid editor layout preset");
}

} // namespace

EditorLayoutWidget::EditorLayoutWidget(std::string identifier, EditorLayout layout)
    : identifier_(std::move(identifier)), layout_(layout) {
    if (identifier_.empty()) throw std::invalid_argument("Editor workspace identifier must not be empty");
    SplitOrder(layout);
    left_ = &Add<Panel>("Left###" + identifier_ + "/left");
    right_ = &Add<Panel>("Right###" + identifier_ + "/right");
    top_ = &Add<Panel>("Top###" + identifier_ + "/top");
    bottom_ = &Add<Panel>("Bottom###" + identifier_ + "/bottom");
    center_ = &Add<Panel>("Center###" + identifier_ + "/center");
}

void EditorLayoutWidget::PreBuild() {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void EditorLayoutWidget::ResetLayout() {
    reset_ = true;
    reopen_ = true;
}

void EditorLayoutWidget::SetLayout(EditorLayout layout) {
    SplitOrder(layout);
    if (layout_ == layout) return;
    layout_ = layout;
    reset_ = true;
}

Panel& EditorLayoutWidget::GetPanel(EditorRegion region) const {
    switch (region) {
    case EditorRegion::Left: return *left_;
    case EditorRegion::Right: return *right_;
    case EditorRegion::Top: return *top_;
    case EditorRegion::Bottom: return *bottom_;
    case EditorRegion::Center: return *center_;
    }
    throw std::invalid_argument("Invalid editor panel region");
}

void EditorLayoutWidget::SetPanelVisible(EditorRegion region, bool visible) {
    auto& panel = GetPanel(region);
    panel.Visible = visible;
    panel.SetOpen(visible);
}

bool EditorLayoutWidget::IsPanelVisible(EditorRegion region) const {
    const auto& panel = GetPanel(region);
    return panel.Visible && panel.IsOpen();
}

void EditorLayoutWidget::Draw() {
    const ImGuiID id = ImHashStr((identifier_ + "/dockspace").c_str());
    if (!Visible) {
        if (ImGui::DockBuilderGetNode(id))
            ImGui::DockSpace(id, ImVec2(0, 0), ImGuiDockNodeFlags_KeepAliveOnly);
        return;
    }
    const auto* viewport = ImGui::GetMainViewport();
    if (reset_ || !ImGui::DockBuilderGetNode(id)) {
        ImGui::DockBuilderRemoveNode(id);
        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(id, viewport->WorkPos);
        ImGui::DockBuilderSetNodeSize(id, viewport->WorkSize);
        ImGuiID remainder = id;
        for (auto region : SplitOrder(layout_)) {
            ImGuiDir direction = ImGuiDir_None;
            float ratio = 0;
            switch (region) {
            case EditorRegion::Top: direction = ImGuiDir_Up; ratio = 0.15f; break;
            case EditorRegion::Bottom: direction = ImGuiDir_Down; ratio = 0.25f; break;
            case EditorRegion::Left: direction = ImGuiDir_Left; ratio = 0.20f; break;
            case EditorRegion::Right: direction = ImGuiDir_Right; ratio = 0.25f; break;
            case EditorRegion::Center: throw std::logic_error("Center cannot be an editor edge split");
            }
            const auto node = ImGui::DockBuilderSplitNode(remainder, direction, ratio, nullptr, &remainder);
            ImGui::DockBuilderDockWindow(GetPanel(region).GetTitle().c_str(), node);
        }
        ImGui::DockBuilderDockWindow(center_->GetTitle().c_str(), remainder);
        ImGui::DockBuilderFinish(id);
        if (reopen_) {
            for (auto* panel : {left_, right_, top_, bottom_, center_}) {
                panel->Visible = true;
                panel->SetOpen(true);
            }
        }
        reset_ = false;
        reopen_ = false;
    }
    ImGui::DockSpaceOverViewport(id, viewport);
    DrawChildren();
}

} // namespace cpptoolkit::ui
