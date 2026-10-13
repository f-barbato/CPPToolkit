#include "TestUIState.h"

#include <cmath>
#include <utility>

TestUIState::StyleEntry::StyleEntry(const char* label, std::function<float&(ImGuiStyle&)> access, float min,
                                    float max, float initial)
    : Label(label), Access(std::move(access)), Min(min), Max(max), Default(initial),
      Value(nullptr, label, initial) {}

TestUIState::TestUIState() {
    const ImGuiStyle defaults;
    const auto scalar = [&](int group, const char* label, float ImGuiStyle::*member, float min, float max) {
        StyleGroup[group].emplace_back(label, [member](ImGuiStyle& s) -> float& { return s.*member; }, min, max,
                                       defaults.*member);
    };
    const auto component = [&](int group, const char* label, ImVec2 ImGuiStyle::*member, float ImVec2::*axis,
                               float max) {
        StyleGroup[group].emplace_back(
            label, [member, axis](ImGuiStyle& s) -> float& { return (s.*member).*axis; }, 0.0f, max,
            (defaults.*member).*axis);
    };
    scalar(0, "Window rounding", &ImGuiStyle::WindowRounding, 0, 16);
    scalar(0, "Child rounding", &ImGuiStyle::ChildRounding, 0, 16);
    scalar(0, "Frame rounding", &ImGuiStyle::FrameRounding, 0, 16);
    scalar(0, "Popup rounding", &ImGuiStyle::PopupRounding, 0, 16);
    scalar(0, "Scrollbar rounding", &ImGuiStyle::ScrollbarRounding, 0, 16);
    scalar(0, "Grab rounding", &ImGuiStyle::GrabRounding, 0, 16);
    scalar(1, "Window border", &ImGuiStyle::WindowBorderSize, 0, 3);
    scalar(1, "Child border", &ImGuiStyle::ChildBorderSize, 0, 3);
    scalar(1, "Popup border", &ImGuiStyle::PopupBorderSize, 0, 3);
    scalar(1, "Frame border", &ImGuiStyle::FrameBorderSize, 0, 3);
    component(2, "Window padding X", &ImGuiStyle::WindowPadding, &ImVec2::x, 30);
    component(2, "Window padding Y", &ImGuiStyle::WindowPadding, &ImVec2::y, 30);
    component(2, "Frame padding X", &ImGuiStyle::FramePadding, &ImVec2::x, 20);
    component(2, "Frame padding Y", &ImGuiStyle::FramePadding, &ImVec2::y, 20);
    component(2, "Item spacing X", &ImGuiStyle::ItemSpacing, &ImVec2::x, 30);
    component(2, "Item spacing Y", &ImGuiStyle::ItemSpacing, &ImVec2::y, 30);
    component(2, "Item inner spacing X", &ImGuiStyle::ItemInnerSpacing, &ImVec2::x, 20);
    component(2, "Item inner spacing Y", &ImGuiStyle::ItemInnerSpacing, &ImVec2::y, 20);
    component(2, "Cell padding X", &ImGuiStyle::CellPadding, &ImVec2::x, 20);
    component(2, "Cell padding Y", &ImGuiStyle::CellPadding, &ImVec2::y, 20);
    scalar(2, "Indent spacing", &ImGuiStyle::IndentSpacing, 0, 50);
    scalar(3, "Scrollbar size", &ImGuiStyle::ScrollbarSize, 4, 30);
    scalar(3, "Grab min size", &ImGuiStyle::GrabMinSize, 4, 30);
    scalar(3, "Docking separator", &ImGuiStyle::DockingSeparatorSize, 0, 10);
    scalar(4, "Alpha", &ImGuiStyle::Alpha, 0.2f, 1);
    scalar(4, "Disabled alpha", &ImGuiStyle::DisabledAlpha, 0, 1);
    scalar(4, "Font scale", &ImGuiStyle::FontScaleMain, 0.5f, 2);
}

void TestUIState::Tick(float dt) {
    elapsed_ += dt;
    Progress.Set((std::sin(elapsed_) + 1.0f) * 0.5f);
    Texture.Set(ImGui::GetIO().Fonts->TexRef.GetTexID());
}
