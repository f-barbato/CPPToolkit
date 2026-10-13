#pragma once

#include <functional>
#include <vector>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/ui.h>

#include "DemoViewModel.h"

namespace demo {

/**
 * @brief Floating controls window, grouped in two tabs.
 *
 * - Appearance: theme, color palette and application font.
 * - Windows: close button and editor layout (presets, panel visibility).
 */
class ControlsPanel : public cpptoolkit::ui::Panel {
public:
    /**
     * @param editor Workspace whose layout and panels are controlled.
     * @param closablePanels Windows affected by the "Allow closing windows" checkbox.
     */
    ControlsPanel(cpptoolkit::ui::EditorLayoutWidget& editor, std::vector<cpptoolkit::ui::Panel*> closablePanels);

protected:
    void OnBuild() override;

private:
    void ApplyStyler(const std::function<void(cpptoolkit::ui::Styler&)>& edit);
    void BuildThemeControls(cpptoolkit::ui::Panel& tab);
    void BuildPaletteControls(cpptoolkit::ui::Panel& tab, DemoViewModel& vm);
    void BuildFontControls(cpptoolkit::ui::Panel& tab, DemoViewModel& vm);
    void BuildCloseControls(cpptoolkit::ui::Panel& tab, DemoViewModel& vm);
    void BuildLayoutControls(cpptoolkit::ui::Panel& tab);

    cpptoolkit::ui::EditorLayoutWidget& editor_;
    std::vector<cpptoolkit::ui::Panel*> closablePanels_;
    cpptoolkit::mvvm::ObservableProperty<int> selectedTab_{nullptr, "ControlsTab", 0};
};

} // namespace demo
