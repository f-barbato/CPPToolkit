#include <string>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cpptoolkit/ui/widgets/FilePathWidget.h>

namespace {

class FilePathWidgetTests : public testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(640.0f, 480.0f);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    void TearDown() override { ImGui::DestroyContext(); }

    void Draw(cpptoolkit::ui::FilePathWidget& widget) {
        ImGui::NewFrame();
        ImGui::Begin("FilePath tests");
        widget.Draw();
        ImGui::End();
        ImGui::Render();
    }
};

TEST_F(FilePathWidgetTests, PollsProgrammaticChangesWithoutCallingHandler) {
    cpptoolkit::mvvm::ObservableProperty<std::string> path{nullptr, "Path", "a.txt"};
    int calls = 0;
    cpptoolkit::ui::FilePathWidget widget("File", path, cpptoolkit::ui::FilePathMode::Save,
                                          {{"Text", "txt"}}, [&](const std::string&) { ++calls; });

    Draw(widget);
    path.Set("b.txt");
    Draw(widget);

    EXPECT_EQ(path.Get(), "b.txt");
    EXPECT_EQ(calls, 0);
    EXPECT_TRUE(widget.LastError().empty());
}

TEST_F(FilePathWidgetTests, ReadOnlyWidgetKeepsValue) {
    cpptoolkit::mvvm::ObservableProperty<std::string> path{nullptr, "Path", "folder"};
    cpptoolkit::ui::FilePathWidget widget("Folder", path, cpptoolkit::ui::FilePathMode::Folder);
    widget.readOnly = true;

    Draw(widget);
    Draw(widget);

    EXPECT_EQ(path.Get(), "folder");
}

} // namespace
