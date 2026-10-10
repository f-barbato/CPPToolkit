#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/widgets/widgets.h>

struct TestUIState : cpptoolkit::mvvm::ObservableObject {
    template <typename T>
    using Property = cpptoolkit::mvvm::ObservableProperty<T>;

    Property<std::string> LastEvent{this, "LastEvent", "Interact with a widget"};
    Property<bool> Checked{this, "Checked", false};
    Property<int> Choice{this, "Choice", 0};
    Property<int> Integer{this, "Integer", 10};
    Property<float> Number{this, "Number", 0.5f};
    Property<std::string> Multiline{this, "Multiline", "Editable multiline text\nSecond line"};
    Property<std::string> Password{this, "Password", ""};
    Property<std::string> Search{this, "Search", ""};
    Property<std::array<float, 4>> Color{this, "Color", {0.2f, 0.6f, 1.0f, 1.0f}};
    Property<cpptoolkit::ui::Date> Date{this, "Date",
        {std::chrono::year{2026}, std::chrono::October, std::chrono::day{10}}};
    Property<cpptoolkit::ui::TimeOfDay> Time{this, "Time", std::chrono::seconds{12 * 3600}};
    Property<cpptoolkit::ui::DateTime> DateTime{this, "DateTime",
        {Date.Get(), Time.Get()}};
    Property<bool> Busy{this, "Busy", true};
    Property<float> Progress{this, "Progress", 0};
    Property<std::string> Badge{this, "Badge", "Connected"};
    Property<bool> NoticeOpen{this, "NoticeOpen", true};
    Property<std::string> Message{this, "Message", "All widgets are bound to this ViewModel"};
    Property<ImTextureID> Texture{this, "Texture", ImTextureID{}};
    Property<bool> Selected{this, "Selected", false};
    Property<int> SelectedRow{this, "SelectedRow", -1};
    Property<int> SelectedNode{this, "SelectedNode", -1};
    Property<int> SelectedTab{this, "SelectedTab", 0};
    Property<int> Breadcrumb{this, "Breadcrumb", 2};
    Property<float> PaneSize{this, "PaneSize", 200};
    Property<bool> PopupOpen{this, "PopupOpen", false};
    Property<bool> DialogOpen{this, "DialogOpen", false};
    Property<bool> ContextOpen{this, "ContextOpen", false};
    Property<cpptoolkit::ui::TableWidget::Rows> Rows{
        this, "Rows", {{"Temperature", "24.8", "C"}, {"Voltage", "3.3", "V"},
                      {"Packets", "128", "count"}}};

    cpptoolkit::mvvm::Command Notify{[this] {
        NoticeOpen.Set(true);
        LastEvent.Set("Notification shown");
    }};
    cpptoolkit::mvvm::Command OpenPopup{[this] { PopupOpen.Set(true); }};
    cpptoolkit::mvvm::Command OpenDialog{[this] { DialogOpen.Set(true); }};
    cpptoolkit::mvvm::Command ClosePopup{[this] { PopupOpen.Set(false); }};
    cpptoolkit::mvvm::Command CloseDialog{[this] { DialogOpen.Set(false); }};
    cpptoolkit::mvvm::Command ContextAction{[this] { LastEvent.Set("Context menu action"); }};

    void Tick(float dt) {
        elapsed_ += dt;
        Progress.Set((std::sin(elapsed_) + 1.0f) * 0.5f);
        Texture.Set(ImGui::GetIO().Fonts->TexRef.GetTexID());
    }

private:
    float elapsed_ = 0;
};

inline void BuildInputExamples(cpptoolkit::ui::Panel& panel, TestUIState& vm) {
    namespace ui = cpptoolkit::ui;
    panel.Add<ui::CheckBoxWidget>("Checked", vm.Checked, [&vm](bool checked) {
        vm.LastEvent.Set(checked ? "Checked" : "Unchecked");
    });
    panel.Add<ui::RadioButtonWidget>("Option A", vm.Choice, 0, [&vm](int) {
        vm.LastEvent.Set("Radio option A");
    });
    panel.Add<ui::RadioButtonWidget>("Option B", vm.Choice, 1, [&vm](int) {
        vm.LastEvent.Set("Radio option B");
    });
    const std::vector<std::string> choices{"Option A", "Option B", "Option C"};
    panel.Add<ui::ComboBoxWidget>("Combo", vm.Choice, choices, [&vm](int choice) {
        vm.LastEvent.Set("Combo selection " + std::to_string(choice));
    });
    panel.Add<ui::ListBoxWidget>("List", vm.Choice, choices, [&vm](int choice) {
        vm.LastEvent.Set("List selection " + std::to_string(choice));
    });
    panel.Add<ui::InputIntWidget>("Integer", vm.Integer).OnValueChanged = [&vm](int value) {
        vm.LastEvent.Set("Integer " + std::to_string(value));
    };
    panel.Add<ui::InputFloatWidget>("Float", vm.Number).OnValueChanged = [&vm](float value) {
        vm.LastEvent.Set("Float " + std::to_string(value));
    };
    panel.Add<ui::DragFloatWidget>("Drag", vm.Number, 0.01f, 0.0f, 1.0f).OnValueChanged =
        [&vm](float value) { vm.LastEvent.Set("Dragged value " + std::to_string(value)); };
    panel.Add<ui::SpinBoxWidget>("Spin", vm.Integer, 1, 0, 100).OnValueChanged =
        [&vm](int value) { vm.LastEvent.Set("Spin value " + std::to_string(value)); };
    panel.Add<ui::TextAreaWidget>("Multiline", vm.Multiline, [&vm](const std::string&) {
        vm.LastEvent.Set("Multiline text changed");
    });
    panel.Add<ui::PasswordWidget>("Password", vm.Password, [&vm](const std::string&) {
        vm.LastEvent.Set("Password changed (content not logged)");
    });
    panel.Add<ui::SearchBoxWidget>("Search", vm.Search, [&vm](const std::string& search) {
        vm.LastEvent.Set("Search: " + search);
    });
    panel.Add<ui::DatePickerWidget>("Date", vm.Date, [&vm](ui::Date) {
        vm.LastEvent.Set("Date changed");
    });
    panel.Add<ui::TimePickerWidget>("Time", vm.Time, [&vm](ui::TimeOfDay) {
        vm.LastEvent.Set("Time changed");
    });
    panel.Add<ui::DateTimePickerWidget>("Date and time", vm.DateTime, [&vm](const ui::DateTime&) {
        vm.LastEvent.Set("Date and time changed");
    });
    panel.Add<ui::ColorPickerWidget>("RGBA", vm.Color, [&vm](const std::array<float, 4>&) {
        vm.LastEvent.Set("Color changed");
    });
}

inline void BuildTestUIWidgets(cpptoolkit::ui::Widget& root, TestUIState& vm) {
    namespace ui = cpptoolkit::ui;
    root.Add<ui::TextWidget>(vm.LastEvent);
    auto& menuBar = root.Add<ui::MenuBarWidget>();
    auto& menu = menuBar.Add<ui::MenuWidget>("Examples");
    menu.Add<ui::MenuItemWidget>("Show notification", vm.Notify);
    menu.Add<ui::MenuItemWidget>("Open dialog", vm.OpenDialog);

    auto& tabs = root.Add<ui::TabBarWidget>("Widget gallery", vm.SelectedTab);
    tabs.OnSelectionChanged = [&vm](int index) {
        vm.LastEvent.Set("Selected gallery tab " + std::to_string(index));
    };
    auto& inputs = tabs.AddTab("Input");
    BuildInputExamples(inputs, vm);

    auto& feedback = tabs.AddTab("Feedback");
    feedback.Add<ui::ProgressBarWidget>("Progress", vm.Progress);
    feedback.Add<ui::SpinnerWidget>("Working", vm.Busy);
    feedback.Add<ui::BadgeWidget>(vm.Badge);
    auto& notice = feedback.Add<ui::NotificationWidget>("Info", vm.Message, vm.NoticeOpen);
    notice.OnDismissed = [&vm] { vm.LastEvent.Set("Notification dismissed"); };
    feedback.Add<ui::ButtonWidget>("Show notification", vm.Notify);
    feedback.Add<ui::TextWidget>(vm.Message);
    feedback.Add<ui::ImageWidget>(vm.Texture, ImVec2(256, 128));

    auto& data = tabs.AddTab("Data");
    data.Add<ui::SelectableWidget>("Selectable item", vm.Selected, [&vm](bool value) {
        vm.LastEvent.Set(value ? "Item selected" : "Item deselected");
    });
    auto& table = data.Add<ui::TableWidget>("Telemetry", std::vector<std::string>{"Name", "Value", "Unit"},
                                           vm.Rows, vm.SelectedRow, true);
    table.OnSelectionChanged = [&vm](int row) {
        vm.LastEvent.Set("Selected row " + std::to_string(row));
    };
    table.OnSortRequested = [&vm](const std::vector<ui::TableSortColumn>& sort) {
        auto rows = vm.Rows.Get();
        std::stable_sort(rows.begin(), rows.end(), [&](const auto& left, const auto& right) {
            for (const auto& column : sort) {
                const auto index = static_cast<std::size_t>(column.Column);
                if (left[index] != right[index])
                    return column.Ascending ? left[index] < right[index] : left[index] > right[index];
            }
            return false;
        });
        vm.Rows.Set(rows);
        vm.SelectedRow.Set(-1);
        vm.LastEvent.Set("Table sort requested");
    };
    data.Add<ui::TreeViewWidget>("Device tree", std::vector<ui::TreeNode>{
        {0, "Board", {{1, "Registers", {{2, "GPIO", {}}, {3, "UART", {}}}},
                      {4, "Memory", {}}}}}, vm.SelectedNode,
        [&vm](int id) { vm.LastEvent.Set("Selected tree node " + std::to_string(id)); });

    auto& navigation = tabs.AddTab("Navigation");
    auto& toolbar = navigation.Add<ui::ToolbarWidget>();
    toolbar.Add<ui::ButtonWidget>("Notify", vm.Notify);
    toolbar.Add<ui::ButtonWidget>("Dialog", vm.OpenDialog);
    navigation.Add<ui::BreadcrumbWidget>(std::vector<std::string>{"Workspace", "Board", "Registers"},
        vm.Breadcrumb, [&vm](int index) {
            vm.LastEvent.Set("Breadcrumb segment " + std::to_string(index));
        });

    auto& layouts = tabs.AddTab("Layout");
    auto& horizontal = layouts.Add<ui::StackPanel>(ui::Orientation::Horizontal);
    horizontal.Add<ui::ButtonWidget>("Horizontal A", vm.Notify);
    horizontal.Add<ui::ButtonWidget>("Horizontal B", vm.Notify);
    auto& vertical = layouts.Add<ui::StackPanel>(ui::Orientation::Vertical);
    vertical.Add<ui::TextWidget>(vm.Message);
    vertical.Add<ui::TextWidget>(vm.Badge);
    auto& grid = layouts.Add<ui::GridPanel>("Two-column grid", 2);
    grid.Add<ui::ButtonWidget>("Grid A", vm.Notify);
    grid.Add<ui::ButtonWidget>("Grid B", vm.Notify);
    auto& scroll = layouts.Add<ui::ScrollPanel>("Scroll area", ImVec2(0, 100));
    for (int i = 0; i < 12; ++i) scroll.Add<ui::TextWidget>(vm.Message);
    auto& splitter = layouts.Add<ui::SplitterWidget>("Drag divider", vm.PaneSize);
    splitter.First().Add<ui::TextWidget>(vm.Badge);
    splitter.Second().Add<ui::TextWidget>(vm.Message);
    splitter.OnSizeChanged = [&vm](float size) {
        vm.LastEvent.Set("Pane width " + std::to_string(size));
    };

    auto& overlays = tabs.AddTab("Overlay");
    overlays.Add<ui::ButtonWidget>("Hover for tooltip", vm.Notify);
    overlays.Add<ui::TooltipWidget>().Add<ui::TextWidget>(vm.Message);
    overlays.Add<ui::ButtonWidget>("Open popup", vm.OpenPopup);
    auto& popup = overlays.Add<ui::PopupWidget>("Popup example", vm.PopupOpen);
    popup.Add<ui::TextWidget>(vm.Message);
    popup.Add<ui::ButtonWidget>("Close popup", vm.ClosePopup);
    popup.OnClosed = [&vm] { vm.LastEvent.Set("Popup dismissed"); };
    overlays.Add<ui::ButtonWidget>("Open modal", vm.OpenDialog);

    // Keep modals at root scope so commands from menus or other tabs can open them.
    auto& dialog = root.Add<ui::DialogWidget>("Modal dialog", vm.DialogOpen);
    dialog.Add<ui::TextWidget>(vm.Message);
    dialog.Add<ui::ButtonWidget>("Close dialog", vm.CloseDialog);
    dialog.OnClosed = [&vm] { vm.LastEvent.Set("Dialog dismissed"); };

    overlays.Add<ui::ButtonWidget>("Right-click for context menu", vm.Notify);
    auto& context = overlays.Add<ui::ContextMenuWidget>("Context example", vm.ContextOpen);
    context.Add<ui::MenuItemWidget>("Notify", vm.Notify);
    context.Add<ui::MenuItemWidget>("Example action", vm.ContextAction);
    context.OnOpened = [&vm] { vm.LastEvent.Set("Context menu opened"); };
    context.OnClosed = [&vm] { vm.LastEvent.Set("Context menu closed"); };
}
