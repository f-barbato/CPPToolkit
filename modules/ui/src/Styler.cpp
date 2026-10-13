#include "cpptoolkit/ui/Styler.h"

#include <stdexcept>

namespace cpptoolkit::ui {

Styler::Styler(Theme theme) {
    SetTheme(theme);
}

Styler& Styler::SetTheme(Theme theme) {
    switch (theme) {
    case Theme::Dark: ImGui::StyleColorsDark(&style_); break;
    case Theme::Light: ImGui::StyleColorsLight(&style_); break;
    case Theme::Classic: ImGui::StyleColorsClassic(&style_); break;
    default: throw std::invalid_argument("Unsupported ImGui theme");
    }
    theme_ = theme;
    return *this;
}

namespace {

ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, 1.0f};
}

ImVec4 Alpha(ImVec4 c, float alpha) {
    c.w = alpha;
    return c;
}

} // namespace

Styler& Styler::SetPalette(const Palette& p) {
    palette_ = p;
    auto* c = style_.Colors;
    const ImVec4 clear = Alpha(p.Background, 0.0f);
    const ImVec4 hover = Mix(p.Surface, p.Primary, 0.30f);
    const ImVec4 active = Mix(p.Surface, p.Primary, 0.50f);
    const ImVec4 button = Mix(p.Surface, p.Primary, 0.65f);
    const ImVec4 buttonHover = p.Primary;
    const ImVec4 buttonActive = Mix(p.Primary, p.Text, 0.25f);
    const ImVec4 header = Mix(p.Surface, p.Secondary, 0.35f);

    c[ImGuiCol_Text] = p.Text;
    c[ImGuiCol_TextDisabled] = p.TextDisabled;
    c[ImGuiCol_WindowBg] = p.Background;
    c[ImGuiCol_ChildBg] = clear;
    c[ImGuiCol_PopupBg] = p.Surface;
    c[ImGuiCol_Border] = p.Border;
    c[ImGuiCol_BorderShadow] = clear;
    c[ImGuiCol_FrameBg] = p.Surface;
    c[ImGuiCol_FrameBgHovered] = hover;
    c[ImGuiCol_FrameBgActive] = active;
    c[ImGuiCol_TitleBg] = p.Surface;
    c[ImGuiCol_TitleBgActive] = active;
    c[ImGuiCol_TitleBgCollapsed] = Alpha(p.Surface, 0.7f);
    c[ImGuiCol_MenuBarBg] = p.Surface;
    c[ImGuiCol_ScrollbarBg] = Alpha(p.Background, 0.6f);
    c[ImGuiCol_ScrollbarGrab] = Mix(p.Surface, p.Text, 0.25f);
    c[ImGuiCol_ScrollbarGrabHovered] = Mix(p.Surface, p.Text, 0.40f);
    c[ImGuiCol_ScrollbarGrabActive] = Mix(p.Surface, p.Text, 0.55f);
    c[ImGuiCol_CheckMark] = p.Primary;
    c[ImGuiCol_SliderGrab] = p.Primary;
    c[ImGuiCol_SliderGrabActive] = p.Secondary;
    c[ImGuiCol_Button] = button;
    c[ImGuiCol_ButtonHovered] = buttonHover;
    c[ImGuiCol_ButtonActive] = buttonActive;
    c[ImGuiCol_Header] = header;
    c[ImGuiCol_HeaderHovered] = Mix(p.Surface, p.Secondary, 0.55f);
    c[ImGuiCol_HeaderActive] = Mix(p.Surface, p.Secondary, 0.75f);
    c[ImGuiCol_Separator] = p.Border;
    c[ImGuiCol_SeparatorHovered] = p.Primary;
    c[ImGuiCol_SeparatorActive] = buttonActive;
    c[ImGuiCol_ResizeGrip] = Alpha(p.Primary, 0.25f);
    c[ImGuiCol_ResizeGripHovered] = Alpha(p.Primary, 0.65f);
    c[ImGuiCol_ResizeGripActive] = Alpha(p.Primary, 0.95f);
    c[ImGuiCol_Tab] = Mix(p.Surface, p.Primary, 0.20f);
    c[ImGuiCol_TabHovered] = p.Primary;
    c[ImGuiCol_TabSelected] = Mix(p.Surface, p.Primary, 0.70f);
    c[ImGuiCol_TabDimmed] = p.Surface;
    c[ImGuiCol_TabDimmedSelected] = Mix(p.Surface, p.Primary, 0.40f);
    c[ImGuiCol_DockingPreview] = Alpha(p.Primary, 0.6f);
    c[ImGuiCol_DockingEmptyBg] = p.Background;
    c[ImGuiCol_PlotLines] = p.Secondary;
    c[ImGuiCol_PlotLinesHovered] = Mix(p.Secondary, p.Text, 0.35f);
    c[ImGuiCol_PlotHistogram] = p.Primary;
    c[ImGuiCol_PlotHistogramHovered] = Mix(p.Primary, p.Text, 0.35f);
    c[ImGuiCol_TableHeaderBg] = p.Surface;
    c[ImGuiCol_TableBorderStrong] = p.Border;
    c[ImGuiCol_TableBorderLight] = Mix(p.Background, p.Border, 0.5f);
    c[ImGuiCol_TableRowBg] = clear;
    c[ImGuiCol_TableRowBgAlt] = Alpha(p.Text, 0.04f);
    c[ImGuiCol_TextSelectedBg] = Alpha(p.Primary, 0.35f);
    c[ImGuiCol_DragDropTarget] = p.Secondary;
    c[ImGuiCol_NavCursor] = p.Primary;
    c[ImGuiCol_NavWindowingHighlight] = Alpha(p.Text, 0.7f);
    c[ImGuiCol_NavWindowingDimBg] = Alpha(p.Background, 0.5f);
    c[ImGuiCol_ModalWindowDimBg] = Alpha(p.Background, 0.55f);
    return *this;
}

void Styler::Apply() const {
    if (!ImGui::GetCurrentContext())
        throw std::logic_error("Styler::Apply requires a current ImGui context");
    ImGui::GetStyle() = style_;
}

} // namespace cpptoolkit::ui
