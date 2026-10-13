#include <gtest/gtest.h>
#include <imgui.h>

#include <cpptoolkit/ui/Shortcuts.h>

namespace {

using cpptoolkit::ui::ShortcutInfo;
using cpptoolkit::ui::ShortcutManager;

class ShortcutTests : public testing::Test {
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

    // Runs one frame after queueing at most one key event (ImGui trickles inputs per frame).
    void Frame(ShortcutManager& manager, ImGuiKey key = ImGuiKey_None, bool down = false) {
        if (key != ImGuiKey_None) ImGui::GetIO().AddKeyEvent(key, down);
        ImGui::NewFrame();
        ImGui::Begin("Shortcut tests");
        manager.Process();
        ImGui::End();
        ImGui::Render();
    }
    void Press(ShortcutManager& manager, ImGuiKey mod, ImGuiKey key) {
        if (mod != ImGuiKey_None) Frame(manager, mod, true);
        Frame(manager, key, true);
        Frame(manager, key, false);
        if (mod != ImGuiKey_None) Frame(manager, mod, false);
    }
};

TEST_F(ShortcutTests, FiresActionOncePerPress) {
    ShortcutManager manager;
    int calls = 0;
    manager.Register({"file.save", "Save", ImGuiMod_Ctrl | ImGuiKey_S, [&] { ++calls; }, nullptr, false, false});
    Press(manager, ImGuiMod_Ctrl, ImGuiKey_S);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(manager.Text("file.save"), "Ctrl+S");
}

TEST_F(ShortcutTests, RespectsCanExecuteAndEnabled) {
    ShortcutManager manager;
    int calls = 0;
    bool allowed = false;
    manager.Register({"a", "", ImGuiMod_Ctrl | ImGuiKey_A, [&] { ++calls; }, [&] { return allowed; }, false, false});
    Press(manager, ImGuiMod_Ctrl, ImGuiKey_A);
    EXPECT_EQ(calls, 0);
    allowed = true;
    manager.Enabled = false;
    Press(manager, ImGuiMod_Ctrl, ImGuiKey_A);
    EXPECT_EQ(calls, 0);
    manager.Enabled = true;
    Press(manager, ImGuiMod_Ctrl, ImGuiKey_A);
    EXPECT_EQ(calls, 1);
}

TEST_F(ShortcutTests, RejectsInvalidDuplicateAndConflictingRegistrations) {
    ShortcutManager manager;
    const auto noop = [] {};
    manager.Register({"a", "", ImGuiMod_Ctrl | ImGuiKey_A, noop, nullptr, false, false});
    EXPECT_THROW(manager.Register({"a", "", ImGuiKey_F1, noop, nullptr, false, false}), std::invalid_argument);
    EXPECT_THROW(manager.Register({"b", "", ImGuiMod_Ctrl | ImGuiKey_A, noop, nullptr, false, false}),
                 std::invalid_argument);
    EXPECT_THROW(manager.Register({"", "", ImGuiKey_F1, noop, nullptr, false, false}), std::invalid_argument);
    EXPECT_THROW(manager.Register({"c", "", ImGuiKey_None, noop, nullptr, false, false}), std::invalid_argument);
    EXPECT_THROW(manager.Register({"d", "", ImGuiKey_F1, nullptr, nullptr, false, false}), std::invalid_argument);
}

TEST_F(ShortcutTests, RebindUnregisterAndCommandBinding) {
    ShortcutManager manager;
    cpptoolkit::mvvm::Command command([] {}, [] { return false; });
    int calls = 0;
    manager.Register({"a", "", ImGuiKey_F1, [&] { ++calls; }, nullptr, false, false});
    manager.Register("cmd", ImGuiKey_F2, command);
    EXPECT_FALSE(manager.Rebind("a", ImGuiKey_F2));
    EXPECT_FALSE(manager.Rebind("missing", ImGuiKey_F3));
    EXPECT_TRUE(manager.Rebind("a", ImGuiKey_F3));
    Press(manager, ImGuiKey_None, ImGuiKey_F1);
    EXPECT_EQ(calls, 0);
    Press(manager, ImGuiKey_None, ImGuiKey_F3);
    EXPECT_EQ(calls, 1);
    EXPECT_TRUE(manager.Unregister("a"));
    EXPECT_FALSE(manager.Unregister("a"));
    EXPECT_EQ(manager.Find("a"), nullptr);
    EXPECT_EQ(manager.FindByChord(ImGuiKey_F2)->Id, "cmd");
}

} // namespace
