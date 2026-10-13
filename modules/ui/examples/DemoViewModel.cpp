#include "DemoViewModel.h"

#include <cmath>
#include <utility>

namespace demo {

using namespace cpptoolkit;

DemoViewModel::PaletteEntry::PaletteEntry(const char* label, ImVec4 ui::Palette::*member, const Color& initial)
    : Label(label), Member(member), Value(nullptr, label, initial) {}

DemoViewModel::DemoViewModel() {
    const ui::Palette palette = ui::Palette::Dark();
    for (const auto& [label, member] : {
             std::pair{"Primary", &ui::Palette::Primary},
             std::pair{"Secondary", &ui::Palette::Secondary},
             std::pair{"Background", &ui::Palette::Background},
             std::pair{"Surface", &ui::Palette::Surface},
             std::pair{"Text", &ui::Palette::Text},
             std::pair{"Text disabled", &ui::Palette::TextDisabled},
             std::pair{"Border", &ui::Palette::Border}}) {
        const ImVec4& c = palette.*member;
        PaletteColors.emplace_back(label, member, Color{c.x, c.y, c.z, c.w});
    }
}

ui::Palette DemoViewModel::CurrentPalette() {
    ui::Palette palette;
    for (auto& entry : PaletteColors) {
        const auto c = entry.Value.Get();
        palette.*entry.Member = ImVec4(c[0], c[1], c[2], c[3]);
    }
    return palette;
}

void DemoViewModel::LoadPalette(const ui::Palette& palette) {
    for (auto& entry : PaletteColors) {
        const ImVec4& c = palette.*entry.Member;
        entry.Value.Set(Color{c.x, c.y, c.z, c.w});
    }
}

void DemoViewModel::Tick(float dt) {
    phase_ += dt;
    SensorValue.Set(Amplitude.Get() * std::sin(phase_));
    Gallery.Tick(dt);
}

} // namespace demo
