#include "ControlsPanel.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace demo {

using namespace cpptoolkit;

ControlsPanel::ControlsPanel(ui::EditorLayoutWidget& editor, std::vector<ui::Panel*> closablePanels)
    : ui::Panel("Controls"), editor_(editor), closablePanels_(std::move(closablePanels)) {}

void ControlsPanel::OnBuild() {
    auto* vm = GetViewModel<DemoViewModel>();
    if (!vm) throw std::logic_error("ControlsPanel requires DemoViewModel");
    closablePanels_.push_back(this);
    auto& tabs = Add<ui::TabBarWidget>("Controls sections", selectedTab_);
    auto& appearance = tabs.AddTab("Appearance");
    BuildThemeControls(appearance);
    BuildPaletteControls(appearance, *vm);
    BuildFontControls(appearance, *vm);
    auto& windows = tabs.AddTab("Windows");
    BuildCloseControls(windows, *vm);
    BuildLayoutControls(windows);
}

void ControlsPanel::ApplyStyler(const std::function<void(ui::Styler&)>& edit) {
    auto styler = GetApplication()->GetStyler();
    edit(styler);
    GetApplication()->SetStyler(styler);
}

void ControlsPanel::BuildThemeControls(ui::Panel& tab) {
    tab.Add<ui::TextWidget>("Global ImGui theme");
    for (const auto& [label, theme] : {
             std::pair{"Dark theme", ui::Theme::Dark},
             std::pair{"Light theme", ui::Theme::Light},
             std::pair{"Classic theme", ui::Theme::Classic}}) {
        tab.Add<ui::ButtonWidget>(label, [this, theme = theme] {
            ApplyStyler([theme](ui::Styler& styler) { styler.SetTheme(theme); });
        });
    }
}

void ControlsPanel::BuildPaletteControls(ui::Panel& tab, DemoViewModel& vm) {
    tab.Add<ui::TextWidget>("Color palette");
    const auto applyPalette = [this, &vm] {
        ApplyStyler([&vm](ui::Styler& styler) { styler.SetPalette(vm.CurrentPalette()); });
    };
    const ui::Palette teal{{0.10f, 0.65f, 0.55f, 1}, {0.95f, 0.55f, 0.15f, 1}, {0.07f, 0.10f, 0.10f, 1},
                           {0.12f, 0.17f, 0.17f, 1}, {0.92f, 0.95f, 0.95f, 1}, {0.48f, 0.55f, 0.55f, 1},
                           {0.22f, 0.30f, 0.30f, 1}};
    for (const auto& [label, preset] : {
             std::pair{"Dark palette", ui::Palette::Dark()},
             std::pair{"Light palette", ui::Palette::Light()},
             std::pair{"Teal palette", teal}}) {
        tab.Add<ui::ButtonWidget>(label, [&vm, preset = preset, applyPalette] {
            vm.LoadPalette(preset);
            applyPalette();
        });
    }
    // One picker per color; every user edit restyles the whole UI.
    for (auto& entry : vm.PaletteColors) {
        tab.Add<ui::ColorPickerWidget>(entry.Label, entry.Value,
                                       [applyPalette](const ui::ColorPickerWidget::Color&) { applyPalette(); });
    }
}

void ControlsPanel::BuildFontControls(ui::Panel& tab, DemoViewModel& vm) {
    tab.Add<ui::TextWidget>("Application font");
    tab.Add<ui::FilePathWidget>("Font file", vm.FontPath, ui::FilePathMode::Open,
                                std::vector<ui::FileFilter>{{"Fonts", "ttf,otf"}});
    tab.Add<ui::DragFloatWidget>("Font size", vm.FontSize, 0.5f, 8.0f, 48.0f);
    tab.Add<ui::ButtonWidget>("Apply font", [this, &vm] {
        try {
            GetApplication()->SetFont(vm.FontPath.Get(), vm.FontSize.Get());
            vm.Status.Set("Font applied");
        } catch (const std::exception& e) {
            vm.Status.Set(e.what());
        }
    });
    tab.Add<ui::ButtonWidget>("Default font", [this, &vm] {
        GetApplication()->ResetFont();
        vm.Status.Set("Default font restored");
    });
}

void ControlsPanel::BuildCloseControls(ui::Panel& tab, DemoViewModel& vm) {
    tab.Add<ui::TextWidget>("Window close button");
    tab.Add<ui::CheckBoxWidget>("Allow closing windows", vm.Closable, [this](bool closable) {
        for (auto* panel : closablePanels_) panel->SetClosable(closable);
    });
}

void ControlsPanel::BuildLayoutControls(ui::Panel& tab) {
    tab.Add<ui::TextWidget>("Layout presets");
    for (const auto& [label, layout] : {
             std::pair{"Full-height left", ui::EditorLayout::LeftFullHeight},
             std::pair{"Full-height right", ui::EditorLayout::RightFullHeight},
             std::pair{"Full-height sidebars", ui::EditorLayout::SidebarsFullHeight},
             std::pair{"Full-width top and bottom", ui::EditorLayout::TopBottomFullWidth}}) {
        tab.Add<ui::ButtonWidget>(label, [this, layout = layout] { editor_.SetLayout(layout); });
    }
    tab.Add<ui::TextWidget>("Show / hide panels");
    for (const auto& [label, region] : {
             std::pair{"Toggle left", ui::EditorRegion::Left},
             std::pair{"Toggle right", ui::EditorRegion::Right},
             std::pair{"Toggle top", ui::EditorRegion::Top},
             std::pair{"Toggle bottom", ui::EditorRegion::Bottom},
             std::pair{"Toggle center", ui::EditorRegion::Center}}) {
        tab.Add<ui::ButtonWidget>(label, [this, region = region] {
            editor_.SetPanelVisible(region, !editor_.IsPanelVisible(region));
        });
    }
    tab.Add<ui::ButtonWidget>("Reset layout and show all panels", [this] { editor_.ResetLayout(); });
}

} // namespace demo
