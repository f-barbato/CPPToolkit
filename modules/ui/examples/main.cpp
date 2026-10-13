#include <cpptoolkit/ui/ui.h>

#include "DemoPanels.h"
#include "DemoViewModel.h"

using namespace cpptoolkit;

namespace {

/** @brief Root view: advances the shared ViewModel once per frame and hosts the editor workspace. */
class EditorView : public ui::View<demo::DemoViewModel> {
public:
    void Render() override {
        if (!RenderEnabled) return;
        _viewModel->Tick(GetFrameTime());
        Widget::Render();
    }

protected:
    void OnBuild() override { Add<demo::EditorPanel>(); }
};

} // namespace

int main() {
    auto& app = ui::Application::Create<EditorView>();
    app.SetTitle("CPPToolkit TestUI")
       .SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI)
       .SetInitialSize(1200, 900)
       .SetMinSize(800, 600)
       .SetTargetFPS(144)
       .SetBackgroundColor(DARKGRAY)
       .SetDarkTheme(true);
    return app.Run();
}
