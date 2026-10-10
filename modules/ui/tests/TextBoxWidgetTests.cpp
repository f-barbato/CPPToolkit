#include <string>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cpptoolkit/ui/widgets/TextBoxWidget.h>

namespace {

class TextBoxWidgetTests : public testing::Test {
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

    void TearDown() override {
        ImGui::DestroyContext();
    }

    void Draw(cpptoolkit::ui::TextBoxWidget& widget, bool focus = false) {
        ImGui::NewFrame();
        ImGui::Begin("TextBox tests");
        if (focus) {
            ImGui::SetKeyboardFocusHere();
        }
        widget.Draw();
        ImGui::End();
        ImGui::Render();
    }

    void Focus(cpptoolkit::ui::TextBoxWidget& widget) {
        Draw(widget, true);
        Draw(widget);
        Draw(widget);
        ASSERT_NE(ImGui::GetActiveID(), 0u);
    }

    void Press(cpptoolkit::ui::TextBoxWidget& widget, ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key, true);
        Draw(widget);
        ImGui::GetIO().AddKeyEvent(key, false);
        Draw(widget);
    }
};

TEST_F(TextBoxWidgetTests, PollsProgrammaticChangesWithoutCallingHandler) {
    cpptoolkit::mvvm::ObservableProperty<std::string> text{nullptr, "Text", "Initial"};
    int calls = 0;
    cpptoolkit::ui::TextBoxWidget widget("Text", text);
    widget.OnTextChanged = [&](const std::string&) { ++calls; };

    Draw(widget);
    text.Set("Updated from ViewModel");
    Focus(widget);

    EXPECT_EQ(text.Get(), "Updated from ViewModel");
    EXPECT_EQ(calls, 0);
    const auto* state = ImGui::GetInputTextState(ImGui::GetActiveID());
    ASSERT_NE(state, nullptr);
    EXPECT_STREQ(state->TextA.Data, "Updated from ViewModel");
}

TEST_F(TextBoxWidgetTests, UserEditUpdatesPropertyBeforeCallingHandler) {
    cpptoolkit::mvvm::ObservableProperty<std::string> text{nullptr, "Text", ""};
    int calls = 0;
    std::string notified;
    cpptoolkit::ui::TextBoxWidget widget("Text", text, [&](const std::string& value) {
        EXPECT_EQ(text.Get(), value);
        notified = value;
        ++calls;
    });
    Focus(widget);

    ImGui::GetIO().AddInputCharactersUTF8("hello");
    Draw(widget);
    Draw(widget);

    EXPECT_EQ(text.Get(), "hello");
    EXPECT_EQ(notified, "hello");
    EXPECT_EQ(calls, 1);
}

TEST_F(TextBoxWidgetTests, HandlesLongUtf8InputAndClearing) {
    cpptoolkit::mvvm::ObservableProperty<std::string> text{nullptr, "Text", ""};
    int calls = 0;
    std::string notified;
    cpptoolkit::ui::TextBoxWidget widget("Text", text, [&](const std::string& value) {
        notified = value;
        ++calls;
    });
    Focus(widget);

    const std::string input = std::string(4096, 'a') + "\xc3\xa8\xe6\x96\x87";
    ImGui::GetIO().AddInputCharactersUTF8(input.c_str());
    Draw(widget);

    EXPECT_EQ(text.Get(), input);
    EXPECT_EQ(notified, input);
    EXPECT_EQ(calls, 1);

    ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, true);
    Press(widget, ImGuiKey_A);
    ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, false);
    Draw(widget);
    Press(widget, ImGuiKey_Backspace);

    EXPECT_TRUE(text.Get().empty());
    EXPECT_TRUE(notified.empty());
    EXPECT_EQ(calls, 2);
}

TEST_F(TextBoxWidgetTests, UserEditWorksWithoutHandler) {
    cpptoolkit::mvvm::ObservableProperty<std::string> text{nullptr, "Text", ""};
    cpptoolkit::ui::TextBoxWidget widget("Text", text);
    Focus(widget);

    ImGui::GetIO().AddInputCharactersUTF8("no callback");
    Draw(widget);

    EXPECT_EQ(text.Get(), "no callback");
}

TEST_F(TextBoxWidgetTests, EditsLongInitialValueAcrossFrames) {
    const std::string initial(4096, 'x');
    cpptoolkit::mvvm::ObservableProperty<std::string> text{nullptr, "Text", initial};
    int calls = 0;
    cpptoolkit::ui::TextBoxWidget widget("Text", text, [&](const std::string& value) {
        EXPECT_EQ(text.Get(), value);
        ++calls;
    });
    Focus(widget);
    Press(widget, ImGuiKey_End);

    ImGui::GetIO().AddInputCharactersUTF8("first");
    Draw(widget);
    ImGui::GetIO().AddInputCharactersUTF8("second");
    Draw(widget);

    EXPECT_EQ(text.Get(), initial + "firstsecond");
    EXPECT_EQ(calls, 2);
}

} // namespace
