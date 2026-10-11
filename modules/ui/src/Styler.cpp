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

void Styler::Apply() const {
    if (!ImGui::GetCurrentContext())
        throw std::logic_error("Styler::Apply requires a current ImGui context");
    ImGui::GetStyle() = style_;
}

} // namespace cpptoolkit::ui
