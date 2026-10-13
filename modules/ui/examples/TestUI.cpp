#include "TestUI.h"

#include <algorithm>
#include <string>
#include <vector>

#include <cpptoolkit/ui/Application.h>
#if defined(CPPTOOLKIT_TESTUI_HAS_CONCURRENCY)
#include "CancellationExample.h"
#endif

namespace ui = cpptoolkit::ui;

namespace {

void BuildFeedbackTab(ui::Panel& tab, TestUIState& vm) {
    tab.Add<ui::ProgressBarWidget>("Progress", vm.Progress);
    tab.Add<ui::SpinnerWidget>("Working", vm.Busy);
    tab.Add<ui::BadgeWidget>(vm.Badge);
    auto& notice = tab.Add<ui::NotificationWidget>("Info", vm.Message, vm.NoticeOpen);
    notice.OnDismissed = [&vm] { vm.LastEvent.Set("Notification dismissed"); };
    tab.Add<ui::ButtonWidget>("Show notification", vm.Notify);
    tab.Add<ui::TextWidget>(vm.Message);
    tab.Add<ui::ImageWidget>(vm.Texture, ImVec2(256, 128));
#if defined(CPPTOOLKIT_TESTUI_HAS_CONCURRENCY)
    tab.Add<CancellationExampleWidget>();
#endif
}

void BuildDataTab(ui::Panel& tab, TestUIState& vm) {
    tab.Add<ui::SelectableWidget>("Selectable item", vm.Selected, [&vm](bool value) {
        vm.LastEvent.Set(value ? "Item selected" : "Item deselected");
    });
    auto& table = tab.Add<ui::TableWidget>("Telemetry", std::vector<std::string>{"Name", "Value", "Unit"},
                                          vm.Rows, vm.SelectedRow, true);
    table.OnSelectionChanged = [&vm](int row) { vm.LastEvent.Set("Selected row " + std::to_string(row)); };
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
    tab.Add<ui::TreeViewWidget>("Device tree", std::vector<ui::TreeNode>{
        {0, "Board", {{1, "Registers", {{2, "GPIO", {}}, {3, "UART", {}}}},
                      {4, "Memory", {}}}}}, vm.SelectedNode,
        [&vm](int id) { vm.LastEvent.Set("Selected tree node " + std::to_string(id)); });
}

void BuildNavigationTab(ui::Panel& tab, TestUIState& vm) {
    auto& toolbar = tab.Add<ui::ToolbarWidget>();
    toolbar.Add<ui::ButtonWidget>("Notify", vm.Notify);
    toolbar.Add<ui::ButtonWidget>("Dialog", vm.OpenDialog);
    tab.Add<ui::BreadcrumbWidget>(std::vector<std::string>{"Workspace", "Board", "Registers"}, vm.Breadcrumb,
        [&vm](int index) { vm.LastEvent.Set("Breadcrumb segment " + std::to_string(index)); });
}

void BuildLayoutTab(ui::Panel& tab, TestUIState& vm) {
    auto& horizontal = tab.Add<ui::StackPanel>(ui::Orientation::Horizontal);
    horizontal.Add<ui::ButtonWidget>("Horizontal A", vm.Notify);
    horizontal.Add<ui::ButtonWidget>("Horizontal B", vm.Notify);
    auto& vertical = tab.Add<ui::StackPanel>(ui::Orientation::Vertical);
    vertical.Add<ui::TextWidget>(vm.Message);
    vertical.Add<ui::TextWidget>(vm.Badge);
    auto& grid = tab.Add<ui::GridPanel>("Two-column grid", 2);
    grid.Add<ui::ButtonWidget>("Grid A", vm.Notify);
    grid.Add<ui::ButtonWidget>("Grid B", vm.Notify);
    auto& scroll = tab.Add<ui::ScrollPanel>("Scroll area", ImVec2(0, 100));
    for (int i = 0; i < 12; ++i) scroll.Add<ui::TextWidget>(vm.Message);
    auto& splitter = tab.Add<ui::SplitterWidget>("Drag divider", vm.PaneSize);
    splitter.First().Add<ui::TextWidget>(vm.Badge);
    splitter.Second().Add<ui::TextWidget>(vm.Message);
    splitter.OnSizeChanged = [&vm](float size) { vm.LastEvent.Set("Pane width " + std::to_string(size)); };
}

// Tab-bar behavior and shape (the gallery's own tab bar is the live preview) followed by global metrics.
void BuildStyleTab(ui::Panel& tab, ui::TabBarWidget& tabs, ui::Widget& root, TestUIState& vm) {
    tab.Add<ui::TextWidget>("Tab bar behavior");
    tab.Add<ui::CheckBoxWidget>("Reorderable tabs", vm.TabsReorderable);
    tab.Add<ui::CheckBoxWidget>("Tab list popup button", vm.TabsListButton);
    tab.Add<ui::ComboBoxWidget>("Overflow policy", vm.TabsFitting,
        std::vector<std::string>{"Shrink tabs", "Scroll tabs"});
    const auto applyTabFlags = [&vm, &tabs] {
        ImGuiTabBarFlags flags = vm.TabsFitting.Get() == 1 ? ImGuiTabBarFlags_FittingPolicyScroll
                                                          : ImGuiTabBarFlags_FittingPolicyShrink;
        if (vm.TabsReorderable.Get()) flags |= ImGuiTabBarFlags_Reorderable;
        if (vm.TabsListButton.Get()) flags |= ImGuiTabBarFlags_TabListPopupButton;
        tabs.SetTabBarFlags(flags);
    };
    applyTabFlags();
    vm.TabsReorderableSub = vm.TabsReorderable.OnChanged([applyTabFlags](const bool&) { applyTabFlags(); });
    vm.TabsListButtonSub = vm.TabsListButton.OnChanged([applyTabFlags](const bool&) { applyTabFlags(); });
    vm.TabsFittingSub = vm.TabsFitting.OnChanged([applyTabFlags](const int&) { applyTabFlags(); });

    tab.Add<ui::TextWidget>("Tab shape");
    // Shape lives in the global style, so each user edit copies the Styler and applies it back.
    const auto applyTabShape = [&vm, &root](float) {
        auto* application = root.GetApplication();
        if (!application) return;
        auto styler = application->GetStyler();
        auto& style = styler.GetStyle();
        style.TabRounding = vm.TabRounding.Get();
        style.TabBorderSize = vm.TabBorderSize.Get();
        style.TabBarOverlineSize = vm.TabOverlineSize.Get();
        style.TabMinWidthBase = vm.TabMinWidth.Get();
        application->SetStyler(styler);
    };
    tab.Add<ui::DragFloatWidget>("Tab rounding", vm.TabRounding, 0.1f, 0.0f, 16.0f, applyTabShape);
    tab.Add<ui::DragFloatWidget>("Tab border", vm.TabBorderSize, 0.05f, 0.0f, 4.0f, applyTabShape);
    tab.Add<ui::DragFloatWidget>("Selected overline", vm.TabOverlineSize, 0.05f, 0.0f, 6.0f, applyTabShape);
    tab.Add<ui::DragFloatWidget>("Minimum tab width", vm.TabMinWidth, 1.0f, 0.0f, 300.0f, applyTabShape);

    const auto applyStyle = [&vm, &root] {
        auto* application = root.GetApplication();
        if (!application) return;
        auto styler = application->GetStyler();
        for (auto& group : vm.StyleGroup)
            for (auto& entry : group) entry.Access(styler.GetStyle()) = entry.Value.Get();
        application->SetStyler(styler);
    };
    for (int g = 0; g < TestUIState::StyleGroupCount; ++g) {
        tab.Add<ui::TextWidget>(TestUIState::StyleGroupNames[g]);
        for (auto& entry : vm.StyleGroup[g]) {
            const float speed = (entry.Max - entry.Min) / 200.0f;
            tab.Add<ui::DragFloatWidget>(entry.Label, entry.Value, speed, entry.Min, entry.Max,
                                         [applyStyle](float) { applyStyle(); });
        }
    }
    tab.Add<ui::ButtonWidget>("Reset style metrics", [&vm, applyStyle] {
        for (auto& group : vm.StyleGroup)
            for (auto& entry : group) entry.Value.Set(entry.Default);
        applyStyle();
    });
}

void BuildOverlayTab(ui::Panel& tab, ui::Widget& root, TestUIState& vm) {
    tab.Add<ui::ButtonWidget>("Hover for tooltip", vm.Notify);
    tab.Add<ui::TooltipWidget>().Add<ui::TextWidget>(vm.Message);
    tab.Add<ui::ButtonWidget>("Open popup", vm.OpenPopup);
    auto& popup = tab.Add<ui::PopupWidget>("Popup example", vm.PopupOpen);
    popup.Add<ui::TextWidget>(vm.Message);
    popup.Add<ui::ButtonWidget>("Close popup", vm.ClosePopup);
    popup.OnClosed = [&vm] { vm.LastEvent.Set("Popup dismissed"); };
    tab.Add<ui::ButtonWidget>("Open modal", vm.OpenDialog);

    // Keep modals at root scope so commands from menus or other tabs can open them.
    auto& dialog = root.Add<ui::DialogWidget>("Modal dialog", vm.DialogOpen);
    dialog.Add<ui::TextWidget>(vm.Message);
    dialog.Add<ui::ButtonWidget>("Close dialog", vm.CloseDialog);
    dialog.OnClosed = [&vm] { vm.LastEvent.Set("Dialog dismissed"); };

    tab.Add<ui::ButtonWidget>("Right-click for context menu", vm.Notify);
    auto& context = tab.Add<ui::ContextMenuWidget>("Context example", vm.ContextOpen);
    context.Add<ui::MenuItemWidget>("Notify", vm.Notify);
    context.Add<ui::MenuItemWidget>("Example action", vm.ContextAction);
    context.OnOpened = [&vm] { vm.LastEvent.Set("Context menu opened"); };
    context.OnClosed = [&vm] { vm.LastEvent.Set("Context menu closed"); };
}

} // namespace

void BuildInputExamples(ui::Panel& panel, TestUIState& vm) {
    panel.Add<ui::CheckBoxWidget>("Checked", vm.Checked, [&vm](bool checked) {
        vm.LastEvent.Set(checked ? "Checked" : "Unchecked");
    });
    panel.Add<ui::RadioButtonWidget>("Option A", vm.Choice, 0, [&vm](int) { vm.LastEvent.Set("Radio option A"); });
    panel.Add<ui::RadioButtonWidget>("Option B", vm.Choice, 1, [&vm](int) { vm.LastEvent.Set("Radio option B"); });
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
    panel.Add<ui::FilePathWidget>("Open file", vm.OpenPath, ui::FilePathMode::Open,
        std::vector<ui::FileFilter>{{"Text", "txt,md"}, {"Images", "png,jpg"}},
        [&vm](const std::string& path) { vm.LastEvent.Set("Open path: " + path); });
    panel.Add<ui::FilePathWidget>("Save file", vm.SavePath, ui::FilePathMode::Save,
        std::vector<ui::FileFilter>{{"Text", "txt"}},
        [&vm](const std::string& path) { vm.LastEvent.Set("Save path: " + path); });
    panel.Add<ui::FilePathWidget>("Folder", vm.FolderPath, ui::FilePathMode::Folder, std::vector<ui::FileFilter>{},
        [&vm](const std::string& path) { vm.LastEvent.Set("Folder: " + path); });
    panel.Add<ui::DatePickerWidget>("Date", vm.Date, [&vm](ui::Date) { vm.LastEvent.Set("Date changed"); });
    panel.Add<ui::TimePickerWidget>("Time", vm.Time, [&vm](ui::TimeOfDay) { vm.LastEvent.Set("Time changed"); });
    panel.Add<ui::DateTimePickerWidget>("Date and time", vm.DateTime, [&vm](const ui::DateTime&) {
        vm.LastEvent.Set("Date and time changed");
    });
    panel.Add<ui::ColorPickerWidget>("RGBA", vm.Color, [&vm](const std::array<float, 4>&) {
        vm.LastEvent.Set("Color changed");
    });
}

void BuildTestUIWidgets(ui::Widget& root, TestUIState& vm) {
    root.Add<ui::TextWidget>(vm.LastEvent);
    auto& menuBar = root.Add<ui::MenuBarWidget>();
    auto& menu = menuBar.Add<ui::MenuWidget>("Examples");
    menu.Add<ui::MenuItemWidget>("Show notification", vm.Notify);
    menu.Add<ui::MenuItemWidget>("Open dialog", vm.OpenDialog);

    auto& tabs = root.Add<ui::TabBarWidget>("Widget gallery", vm.SelectedTab);
    tabs.OnSelectionChanged = [&vm](int index) { vm.LastEvent.Set("Selected gallery tab " + std::to_string(index)); };
    BuildInputExamples(tabs.AddTab("Input"), vm);
    BuildFeedbackTab(tabs.AddTab("Feedback"), vm);
    BuildDataTab(tabs.AddTab("Data"), vm);
    BuildNavigationTab(tabs.AddTab("Navigation"), vm);
    BuildLayoutTab(tabs.AddTab("Layout"), vm);
    BuildStyleTab(tabs.AddTab("Style"), tabs, root, vm);
    BuildOverlayTab(tabs.AddTab("Overlay"), root, vm);
}
