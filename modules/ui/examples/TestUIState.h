#pragma once

#include <array>
#include <chrono>
#include <deque>
#include <functional>
#include <string>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/widgets/widgets.h>

/** @brief ViewModel of the TestUI widget gallery; every control is bound to one of its properties. */
struct TestUIState : cpptoolkit::mvvm::ObservableObject {
    template <typename T>
    using Property = cpptoolkit::mvvm::ObservableProperty<T>;

    /** @brief One global ImGuiStyle metric edited from the Style tab. */
    struct StyleEntry {
        StyleEntry(const char* label, std::function<float&(ImGuiStyle&)> access, float min, float max, float initial);
        const char* Label;
        std::function<float&(ImGuiStyle&)> Access;
        float Min, Max, Default;
        cpptoolkit::mvvm::ObservableProperty<float> Value;
    };

    static constexpr int StyleGroupCount = 5;
    static constexpr const char* StyleGroupNames[StyleGroupCount] = {
        "Rounding", "Borders", "Spacing and padding", "Sizes", "Opacity and scale"};

    TestUIState();

    /** @brief Advance animated values; call once per frame on the render thread. */
    void Tick(float dt);

    // Event log shown above the gallery.
    Property<std::string> LastEvent{this, "LastEvent", "Interact with a widget"};

    // Input tab.
    Property<bool> Checked{this, "Checked", false};
    Property<int> Choice{this, "Choice", 0};
    Property<int> Integer{this, "Integer", 10};
    Property<float> Number{this, "Number", 0.5f};
    Property<std::string> Multiline{this, "Multiline", "Editable multiline text\nSecond line"};
    Property<std::string> Password{this, "Password", ""};
    Property<std::string> Search{this, "Search", ""};
    Property<std::string> OpenPath{this, "OpenPath", ""};
    Property<std::string> SavePath{this, "SavePath", ""};
    Property<std::string> FolderPath{this, "FolderPath", ""};
    Property<std::array<float, 4>> Color{this, "Color", {0.2f, 0.6f, 1.0f, 1.0f}};
    Property<cpptoolkit::ui::Date> Date{this, "Date",
        {std::chrono::year{2026}, std::chrono::October, std::chrono::day{10}}};
    Property<cpptoolkit::ui::TimeOfDay> Time{this, "Time", std::chrono::seconds{12 * 3600}};
    Property<cpptoolkit::ui::DateTime> DateTime{this, "DateTime", {Date.Get(), Time.Get()}};

    // Feedback tab.
    Property<bool> Busy{this, "Busy", true};
    Property<float> Progress{this, "Progress", 0};
    Property<std::string> Badge{this, "Badge", "Connected"};
    Property<bool> NoticeOpen{this, "NoticeOpen", true};
    Property<std::string> Message{this, "Message", "All widgets are bound to this ViewModel"};
    Property<ImTextureID> Texture{this, "Texture", ImTextureID{}};

    // Data tab.
    Property<bool> Selected{this, "Selected", false};
    Property<int> SelectedRow{this, "SelectedRow", -1};
    Property<int> SelectedNode{this, "SelectedNode", -1};
    Property<cpptoolkit::ui::TableWidget::Rows> Rows{
        this, "Rows", {{"Temperature", "24.8", "C"}, {"Voltage", "3.3", "V"}, {"Packets", "128", "count"}}};

    // Navigation and layout tabs.
    Property<int> SelectedTab{this, "SelectedTab", 0};
    Property<int> Breadcrumb{this, "Breadcrumb", 2};
    Property<float> PaneSize{this, "PaneSize", 200};

    // Style tab: tab-bar behavior and shape.
    Property<bool> TabsReorderable{this, "TabsReorderable", false};
    Property<bool> TabsListButton{this, "TabsListButton", false};
    Property<int> TabsFitting{this, "TabsFitting", 0};
    Property<float> TabRounding{this, "TabRounding", 5.0f};
    Property<float> TabBorderSize{this, "TabBorderSize", 0.0f};
    Property<float> TabOverlineSize{this, "TabOverlineSize", 2.0f};
    Property<float> TabMinWidth{this, "TabMinWidth", 0.0f};
    cpptoolkit::mvvm::Observable<bool>::Subscription TabsReorderableSub, TabsListButtonSub;
    cpptoolkit::mvvm::Observable<int>::Subscription TabsFittingSub;

    // Style tab: global ImGuiStyle metrics, grouped by StyleGroupNames.
    std::deque<StyleEntry> StyleGroup[StyleGroupCount];

    // Overlay tab.
    Property<bool> PopupOpen{this, "PopupOpen", false};
    Property<bool> DialogOpen{this, "DialogOpen", false};
    Property<bool> ContextOpen{this, "ContextOpen", false};

    cpptoolkit::mvvm::Command Notify{[this] {
        NoticeOpen.Set(true);
        LastEvent.Set("Notification shown");
    }};
    cpptoolkit::mvvm::Command OpenPopup{[this] { PopupOpen.Set(true); }};
    cpptoolkit::mvvm::Command OpenDialog{[this] { DialogOpen.Set(true); }};
    cpptoolkit::mvvm::Command ClosePopup{[this] { PopupOpen.Set(false); }};
    cpptoolkit::mvvm::Command CloseDialog{[this] { DialogOpen.Set(false); }};
    cpptoolkit::mvvm::Command ContextAction{[this] { LastEvent.Set("Context menu action"); }};

private:
    float elapsed_ = 0;
};
