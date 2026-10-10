#include <cstdlib>
#include <stdexcept>

#include <gtest/gtest.h>
#include <imgui_internal.h>
#include <cpptoolkit/ui/Application.h>
#include <cpptoolkit/ui/View.h>

namespace {

using namespace cpptoolkit;

class RenderingModel : public mvvm::ObservableObject {
public:
    mvvm::ObservableProperty<bool> Red{this, "Red", true};
};

class RenderingRoot : public ui::View<RenderingModel> {
public:
    int Frames = 0;
    int Renders = 0;
    bool DestroyedWithWindow = false;
    bool FailRendering = false;
    bool FailDrawing = false;
protected:
    void OnBuild() override {
        screen_ = &Add<ui::RenderTextureWidget>("Framebuffer", 32, 24,
            [this](ui::RenderTextureWidget& widget) {
                EXPECT_FALSE(ImGui::GetCurrentContext()->WithinFrameScope);
                EXPECT_EQ(widget.GetViewModel<RenderingModel>(), _viewModel.get());
                ++Renders;
                DrawRectangle(0, 0, 32, 12, _viewModel->Red.Get() ? RED : BLUE);
                DrawRectangle(0, 12, 32, 12, GREEN);
                if (FailRendering) throw std::runtime_error("render failure");
            });
    }
public:
    void Draw() override {
        EXPECT_TRUE(ImGui::GetCurrentContext()->WithinFrameScope);
        EXPECT_EQ(Renders, Frames + 1);
        const auto& target = screen_->GetRenderTexture();
        Image pixels = LoadImageFromTexture(target.texture);
        // OpenGL render-target storage is bottom-up; Draw() corrects its UVs.
        const Color bottom = GetImageColor(pixels, 16, 5);
        const Color top = GetImageColor(pixels, 16, 18);
        EXPECT_EQ(bottom.g, (GREEN.g));
        EXPECT_EQ(top.r, (_viewModel->Red.Get() ? RED.r : BLUE.r));
        UnloadImage(pixels);
        ImGui::SetNextWindowSize(ImVec2(160, 160));
        ImGui::Begin("Rendering test");
        const auto firstVertex = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        screen_->Draw();
        const auto& vertices = ImGui::GetWindowDrawList()->VtxBuffer;
        bool flipped = false;
        for (int i = firstVertex; i + 3 < vertices.Size; ++i) {
            if (vertices[i].uv.x == 0 && vertices[i].uv.y == 1 &&
                vertices[i + 2].uv.x == 1 && vertices[i + 2].uv.y == 0) flipped = true;
        }
        EXPECT_TRUE(flipped);
        ImGui::End();
        if (FailDrawing) throw std::runtime_error("draw failure");
        _viewModel->Red.Set(false);
        if (++Frames == 3) GetApplication()->Stop();
    }
    void Destroy() override {
        DestroyedWithWindow = IsWindowReady();
        EXPECT_FALSE(ImGui::GetCurrentContext()->WithinFrameScope);
        ui::Widget::Destroy();
    }
private:
    ui::RenderTextureWidget* screen_ = nullptr;
};

TEST(RaylibRenderingTests, ApplicationRendersSharedModelTextureBeforeGuiAndCleansUpFailures) {
#if defined(__linux__)
    if (!std::getenv("DISPLAY")) GTEST_SKIP() << "Requires an X11 display (use xvfb-run)";
#endif
    for (int failure = 0; failure < 3; ++failure) {
        auto root = std::make_shared<RenderingRoot>();
        root->FailRendering = failure == 1;
        root->FailDrawing = failure == 2;
        ui::Application application(root);
        int errors = 0;
        application.SetConfigFlags(FLAG_WINDOW_HIDDEN)
                   .SetInitialSize(200, 200).SetTargetFPS(0)
                   .SetOnError([&](const std::exception& error) {
                       ++errors;
                       EXPECT_STREQ(error.what(), failure == 1 ? "render failure" : "draw failure");
                   });
        EXPECT_EQ(application.Run(), failure == 0 ? 0 : 1);
        EXPECT_EQ(errors, failure == 0 ? 0 : 1);
        EXPECT_EQ(root->Frames, failure == 0 ? 3 : 0);
        EXPECT_TRUE(root->DestroyedWithWindow);
        EXPECT_FALSE(IsWindowReady());
    }
}

} // namespace
