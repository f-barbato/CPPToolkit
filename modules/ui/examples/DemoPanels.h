#pragma once

#include <vector>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/ui.h>

#include "DemoViewModel.h"

namespace demo {

/**
 * @brief Live demo window, grouped in two tabs.
 *
 * - Telemetry: simulated sensor, amplitude slider and plot.
 * - Editing: undo/redo history (Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z) and text inputs.
 */
class LiveDemoPanel : public cpptoolkit::ui::Panel {
public:
    LiveDemoPanel();

protected:
    void OnBuild() override;

private:
    void RegisterUndoShortcuts(DemoViewModel& vm);

    cpptoolkit::mvvm::ObservableProperty<int> selectedTab_{nullptr, "LiveDemoTab", 0};
};

/** @brief Window hosting the TestUI widget gallery. */
class GalleryPanel : public cpptoolkit::ui::Panel {
public:
    GalleryPanel();

protected:
    void OnBuild() override;
};

/** @brief Dockable editor workspace hosting every demo window. */
class EditorPanel : public cpptoolkit::ui::EditorLayoutWidget {
public:
    EditorPanel();

protected:
    void OnBuild() override;

private:
    void BuildRegions(DemoViewModel& vm);
};

} // namespace demo
