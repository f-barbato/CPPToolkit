#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cpptoolkit/ui/widgets/InputWidgets.h>

namespace {

using namespace cpptoolkit::ui;
using cpptoolkit::mvvm::ObservableProperty;

class InputWidgetsTests : public testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(1000, 800);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    void TearDown() override { ImGui::DestroyContext(); }

    void Draw(Widget& widget, bool focus = false) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::SetNextWindowSize(ImVec2(800, 700));
        ImGui::Begin("Input tests", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (focus) ImGui::SetKeyboardFocusHere();
        widget.Draw();
        itemMin_ = ImGui::GetItemRectMin();
        itemMax_ = ImGui::GetItemRectMax();
        ImGui::End();
        ImGui::Render();
    }

    void Focus(Widget& widget) {
        Draw(widget, true);
        Draw(widget);
        Draw(widget);
        ASSERT_NE(ImGui::GetActiveID(), 0u);
    }

    void Click(Widget& widget, ImVec2 point) {
        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(point.x, point.y);
        Draw(widget);
        io.AddMouseButtonEvent(0, true);
        Draw(widget);
        io.AddMouseButtonEvent(0, false);
        Draw(widget);
    }

    void Press(Widget& widget, ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key, true);
        Draw(widget);
        ImGui::GetIO().AddKeyEvent(key, false);
        Draw(widget);
    }

    void SelectAll(Widget& widget) {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, true);
        Press(widget, ImGuiKey_A);
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, false);
        Draw(widget);
    }

    template <typename T>
    void CheckStringEditing(bool multiline = false) {
        ObservableProperty<std::string> text(nullptr, "Text", "");
        int calls = 0;
        T widget("Text", text, [&](const std::string& value) {
            EXPECT_EQ(text.Get(), value);
            ++calls;
        });
        Focus(widget);
        const std::string input = std::string(4096, 'a') + "\xc3\xa8\xe6\x96\x87";
        ImGui::GetIO().AddInputCharactersUTF8(input.c_str());
        Draw(widget);
        Draw(widget);
        EXPECT_EQ(text.Get(), input);
        EXPECT_EQ(calls, 1);

        if (multiline) {
            Press(widget, ImGuiKey_Enter);
            EXPECT_EQ(text.Get(), input + "\n");
            EXPECT_EQ(calls, 2);
        }

        SelectAll(widget);
        Press(widget, ImGuiKey_Backspace);
        EXPECT_TRUE(text.Get().empty());
        EXPECT_EQ(calls, multiline ? 3 : 2);
    }

    template <typename T>
    void CheckReadOnlyText() {
        const std::string initial = std::string(4096, 'x') + "\xe6\x96\x87";
        ObservableProperty<std::string> text(nullptr, "Text", initial);
        int calls = 0;
        T widget("Read only", text);
        widget.OnTextChanged = [&](const std::string&) { ++calls; };
        widget.readOnly = true;
        Focus(widget);
        ImGui::GetIO().AddInputCharactersUTF8("cannot change");
        Draw(widget);
        Press(widget, ImGuiKey_Backspace);
        EXPECT_EQ(text.Get(), initial);
        EXPECT_EQ(calls, 0);
        text.Set(initial + " programmatic");
        Draw(widget);
        EXPECT_EQ(text.Get(), initial + " programmatic");
        EXPECT_EQ(calls, 0);
    }

    ImVec2 itemMin_{};
    ImVec2 itemMax_{};
};

TEST_F(InputWidgetsTests, CheckboxUserEventCommitsBeforeCallbackAndProgrammaticSetIsSilent) {
    ObservableProperty<bool> checked(nullptr, "Checked", false);
    int calls = 0;
    CheckBoxWidget widget("Check", checked, [&](bool value) {
        EXPECT_EQ(checked.Get(), value);
        ++calls;
    });
    Draw(widget);
    Click(widget, ImVec2(itemMin_.x + 8, itemMin_.y + 8));
    EXPECT_TRUE(checked.Get());
    EXPECT_EQ(calls, 1);
    Draw(widget);
    checked.Set(false);
    Draw(widget);
    EXPECT_FALSE(checked.Get());
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, RadioOnlyNotifiesWhenSelectionActuallyChanges) {
    ObservableProperty<int> selection(nullptr, "Selection", -1);
    int calls = 0;
    RadioButtonWidget widget("Option", selection, 2, [&](int value) {
        EXPECT_EQ(selection.Get(), value);
        ++calls;
    });
    Draw(widget);
    const ImVec2 point(itemMin_.x + 8, itemMin_.y + 8);
    Click(widget, point);
    EXPECT_EQ(selection.Get(), 2);
    EXPECT_EQ(calls, 1);
    Click(widget, point);
    EXPECT_EQ(calls, 1);
    selection.Set(20);
    Draw(widget);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, ComboSelectionCommitsBeforeCallback) {
    ObservableProperty<int> selection(nullptr, "Selection", -1);
    int calls = 0;
    ComboBoxWidget widget("Combo", selection, {"First", "Second"}, [&](int value) {
        EXPECT_EQ(selection.Get(), value);
        ++calls;
    });
    Draw(widget);
    Click(widget, ImVec2(itemMin_.x + 40, itemMin_.y + 8));
    Draw(widget);
    ImGuiWindow* popup = nullptr;
    for (auto* window : ImGui::GetCurrentContext()->Windows) {
        if (window->Active && (window->Flags & ImGuiWindowFlags_Popup)) popup = window;
    }
    ASSERT_NE(popup, nullptr);
    Click(widget, ImVec2(popup->Pos.x + popup->WindowPadding.x + 8,
                        popup->Pos.y + popup->WindowPadding.y + 8));
    EXPECT_EQ(selection.Get(), 0);
    EXPECT_EQ(calls, 1);
    selection.Set(1);
    Draw(widget);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, ListSelectionCommitsBeforeCallbackAndHandlesDuplicateLabels) {
    ObservableProperty<int> selection(nullptr, "Selection", -1);
    int calls = 0;
    ListBoxWidget widget("List", selection, {"Same", "Same"}, [&](int value) {
        EXPECT_EQ(selection.Get(), value);
        ++calls;
    });
    Draw(widget);
    const ImVec2 first(itemMin_.x + 12, itemMin_.y + 10);
    Click(widget, first);
    EXPECT_EQ(selection.Get(), 0);
    EXPECT_EQ(calls, 1);
    Click(widget, first);
    EXPECT_EQ(calls, 1);
    Click(widget, ImVec2(first.x, first.y + ImGui::GetTextLineHeightWithSpacing()));
    EXPECT_EQ(selection.Get(), 1);
    EXPECT_EQ(calls, 2);
}

TEST_F(InputWidgetsTests, SelectorsAcceptNoSelectionAndSafelyPollInvalidDynamicValues) {
    ObservableProperty<int> selected(nullptr, "Selected", -1);
    int calls = 0;
    ComboBoxWidget combo("Combo", selected, {}, [&](int) { ++calls; });
    ListBoxWidget list("List", selected, {}, [&](int) { ++calls; });
    Draw(combo);
    Draw(list);
    selected.Set(200);
    Draw(combo);
    Draw(list);
    selected.Set(-100);
    Draw(combo);
    Draw(list);
    EXPECT_EQ(selected.Get(), -100);
    EXPECT_EQ(calls, 0);
}

TEST_F(InputWidgetsTests, SelectorsRejectInvalidInitialConfiguration) {
    ObservableProperty<int> selected(nullptr, "Selected", -2);
    EXPECT_THROW(ComboBoxWidget("Combo", selected, {"One"}), std::invalid_argument);
    EXPECT_THROW(ListBoxWidget("List", selected, {"One"}), std::invalid_argument);
    EXPECT_THROW(RadioButtonWidget("Radio", selected, 0), std::invalid_argument);
    selected.Set(1);
    EXPECT_THROW(ComboBoxWidget("Combo", selected, {"One"}), std::invalid_argument);
    EXPECT_THROW(ListBoxWidget("List", selected, {}), std::invalid_argument);
    selected.Set(-1);
    EXPECT_THROW(RadioButtonWidget("Radio", selected, -1), std::invalid_argument);
}

TEST_F(InputWidgetsTests, NumericControlsRejectInvalidConfiguration) {
    ObservableProperty<int> integer(nullptr, "Integer", 0);
    ObservableProperty<float> number(nullptr, "Number", 0);
    EXPECT_THROW(InputIntWidget("Int", integer, -1), std::invalid_argument);
    EXPECT_THROW(InputIntWidget("Int", integer, 1, -1), std::invalid_argument);
    EXPECT_THROW(InputFloatWidget("Float", number, -1), std::invalid_argument);
    EXPECT_THROW(InputFloatWidget("Float", number, 0, std::numeric_limits<float>::infinity()),
                 std::invalid_argument);
    EXPECT_THROW(DragFloatWidget("Drag", number, 0), std::invalid_argument);
    EXPECT_THROW(DragFloatWidget("Drag", number, 1, 2, 1), std::invalid_argument);
    EXPECT_THROW(DragFloatWidget("Drag", number, std::numeric_limits<float>::quiet_NaN()),
                 std::invalid_argument);
    EXPECT_THROW(SpinBoxWidget("Spin", integer, 0, 0, 10), std::invalid_argument);
    EXPECT_THROW(SpinBoxWidget("Spin", integer, 1, 10, 0), std::invalid_argument);
}

TEST_F(InputWidgetsTests, IntegerInputCommitsUserEditsBeforeCallback) {
    ObservableProperty<int> value(nullptr, "Value", 0);
    int calls = 0;
    InputIntWidget widget("Integer", value, 1, 100, [&](int edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Focus(widget);
    SelectAll(widget);
    ImGui::GetIO().AddInputCharactersUTF8("42");
    Draw(widget);
    Press(widget, ImGuiKey_Enter);
    EXPECT_EQ(value.Get(), 42);
    EXPECT_EQ(calls, 1);
    Draw(widget);
    value.Set(9);
    Draw(widget);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, FloatInputCommitsUserEditsBeforeCallback) {
    ObservableProperty<float> value(nullptr, "Value", 0);
    int calls = 0;
    InputFloatWidget widget("Float", value, 0, 0, [&](float edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Focus(widget);
    SelectAll(widget);
    ImGui::GetIO().AddInputCharactersUTF8("2.5");
    Draw(widget);
    Press(widget, ImGuiKey_Enter);
    EXPECT_FLOAT_EQ(value.Get(), 2.5f);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, SpinBoxClampsUserInputButNotProgrammaticChanges) {
    ObservableProperty<int> value(nullptr, "Value", 0);
    int calls = 0;
    SpinBoxWidget widget("Spin", value, 2, -10, 10, [&](int edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Focus(widget);
    SelectAll(widget);
    ImGui::GetIO().AddInputCharactersUTF8("100");
    Draw(widget);
    Press(widget, ImGuiKey_Enter);
    EXPECT_EQ(value.Get(), 10);
    EXPECT_EQ(calls, 1);
    value.Set(200);
    Draw(widget);
    EXPECT_EQ(value.Get(), 200);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, SpinButtonsClampWithoutOverflowOrDuplicateCallbacks) {
    ObservableProperty<int> value(nullptr, "Value", std::numeric_limits<int>::max() - 1);
    int calls = 0;
    SpinBoxWidget widget("Spin", value, 10, std::numeric_limits<int>::min(),
                         std::numeric_limits<int>::max(), [&](int edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Draw(widget);
    const ImVec2 plus((itemMin_.x + itemMax_.x) / 2, (itemMin_.y + itemMax_.y) / 2);
    Click(widget, plus);
    EXPECT_EQ(value.Get(), std::numeric_limits<int>::max());
    EXPECT_EQ(calls, 1);
    Click(widget, plus);
    EXPECT_EQ(calls, 1);
}

TEST_F(InputWidgetsTests, DragAndColorPollProgrammaticUpdatesWithoutCallbacks) {
    ObservableProperty<float> number(nullptr, "Number", 1);
    ObservableProperty<ColorPickerWidget::Color> color(nullptr, "Color", {1, 0, 0, 1});
    int calls = 0;
    DragFloatWidget drag("Drag", number, 0.1f, 0, 10, [&](float) { ++calls; });
    ColorPickerWidget picker("Color", color, [&](const auto&) { ++calls; });
    Draw(drag);
    Draw(picker);
    number.Set(5);
    color.Set({0, 1, 0, 0.5f});
    Draw(drag);
    Draw(picker);
    EXPECT_EQ(calls, 0);
}

TEST_F(InputWidgetsTests, DragUserInteractionUpdatesPropertyBeforeCallback) {
    ObservableProperty<float> value(nullptr, "Value", 1);
    int calls = 0;
    DragFloatWidget widget("Drag", value, 0.1f, 0, 10, [&](float edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Draw(widget);
    const ImVec2 start(itemMin_.x + 40, itemMin_.y + 8);
    auto& io = ImGui::GetIO();
    io.AddMousePosEvent(start.x, start.y);
    Draw(widget);
    io.AddMouseButtonEvent(0, true);
    Draw(widget);
    io.AddMousePosEvent(start.x + 80, start.y);
    Draw(widget);
    io.AddMouseButtonEvent(0, false);
    Draw(widget);
    EXPECT_GT(value.Get(), 1);
    EXPECT_LE(value.Get(), 10);
    EXPECT_GT(calls, 0);
}

TEST_F(InputWidgetsTests, ColorPickerUserInteractionUpdatesPropertyBeforeCallback) {
    const ColorPickerWidget::Color initial{1, 0, 0, 1};
    ObservableProperty<ColorPickerWidget::Color> value(nullptr, "Value", initial);
    int calls = 0;
    ColorPickerWidget widget("Color", value, [&](const ColorPickerWidget::Color& edited) {
        EXPECT_EQ(value.Get(), edited);
        ++calls;
    });
    Draw(widget);
    Click(widget, ImVec2(itemMin_.x + 30, itemMin_.y + 30));
    EXPECT_NE(value.Get(), initial);
    EXPECT_GT(calls, 0);
}

TEST_F(InputWidgetsTests, ReadOnlyNumericControlsIgnoreKeyboardAndSpinButtons) {
    ObservableProperty<int> integer(nullptr, "Integer", 3);
    ObservableProperty<float> number(nullptr, "Number", 2.5f);
    int calls = 0;
    InputIntWidget inputInt("Int", integer, 1, 100, [&](int) { ++calls; });
    InputFloatWidget inputFloat("Float", number, 0, 0, [&](float) { ++calls; });
    SpinBoxWidget spin("Spin", integer, 1, 0, 10, [&](int) { ++calls; });
    inputInt.readOnly = true;
    inputFloat.readOnly = true;
    spin.readOnly = true;
    for (Widget* widget : std::vector<Widget*>{&inputInt, &inputFloat}) {
        Focus(*widget);
        SelectAll(*widget);
        ImGui::GetIO().AddInputCharactersUTF8("9");
        Draw(*widget);
        Press(*widget, ImGuiKey_Enter);
    }
    Draw(spin);
    Click(spin, ImVec2((itemMin_.x + itemMax_.x) / 2, (itemMin_.y + itemMax_.y) / 2));
    EXPECT_EQ(integer.Get(), 3);
    EXPECT_FLOAT_EQ(number.Get(), 2.5f);
    EXPECT_EQ(calls, 0);
}

TEST_F(InputWidgetsTests, TextWidgetsWorkWithoutOptionalHandlers) {
    ObservableProperty<std::string> text(nullptr, "Text", "");
    TextAreaWidget area("Area", text);
    PasswordWidget password("Password", text);
    SearchBoxWidget search("Search", text);
    for (Widget* widget : std::vector<Widget*>{&area, &password, &search}) {
        text.Set("");
        Focus(*widget);
        ImGui::GetIO().AddInputCharactersUTF8("value");
        Draw(*widget);
        EXPECT_EQ(text.Get(), "value");
    }
}

TEST_F(InputWidgetsTests, PasswordHandlesLongUtf8AndOnlyNotifiesActualUserChanges) {
    CheckStringEditing<PasswordWidget>();
}

TEST_F(InputWidgetsTests, SearchHandlesLongUtf8AndOnlyNotifiesActualUserChanges) {
    CheckStringEditing<SearchBoxWidget>();
}

TEST_F(InputWidgetsTests, TextAreaHandlesLongUtf8NewlinesAndClearing) {
    CheckStringEditing<TextAreaWidget>(true);
}

TEST_F(InputWidgetsTests, PasswordReadOnlyStillPollsProgrammaticLongUtf8Values) {
    CheckReadOnlyText<PasswordWidget>();
}

TEST_F(InputWidgetsTests, SearchReadOnlyStillPollsProgrammaticLongUtf8Values) {
    CheckReadOnlyText<SearchBoxWidget>();
}

TEST_F(InputWidgetsTests, TextAreaReadOnlyStillPollsProgrammaticLongUtf8Values) {
    CheckReadOnlyText<TextAreaWidget>();
}

TEST_F(InputWidgetsTests, HiddenWidgetsDoNotEmitItemsOrCallbacks) {
    ObservableProperty<bool> value(nullptr, "Value", false);
    CheckBoxWidget widget("Hidden", value, [](bool) { ADD_FAILURE(); });
    widget.Visible = false;
    Draw(widget);
    EXPECT_FALSE(value.Get());
}

} // namespace
