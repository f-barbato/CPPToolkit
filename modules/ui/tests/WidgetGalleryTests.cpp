#include <limits>
#include <stdexcept>
#include <future>
#include <chrono>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include <cpptoolkit/ui/widgets/widgets.h>
#include <cpptoolkit/ui/Application.h>
#include <cpptoolkit/ui/View.h>
#include <cpptoolkit/mvvm/Dispatcher.h>

#include "../examples/TestUI.h"

namespace {

class WidgetGalleryTests : public testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImPlot::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(1000, 900);
        io.DeltaTime = 1.0f / 60;
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        io.Fonts->SetTexID(1);
    }

    void TearDown() override {
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    ImVec2 Draw(cpptoolkit::ui::Widget& widget) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(900, 800));
        ImGui::Begin("Gallery tests", nullptr, ImGuiWindowFlags_MenuBar);
        widget.Draw();
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        ImGui::End();
        ImGui::Render();
        return ImVec2((min.x + max.x) / 2, (min.y + max.y) / 2);
    }

    void Click(cpptoolkit::ui::Widget& widget, ImVec2 at, int button = 0) {
        ImGui::GetIO().AddMousePosEvent(at.x, at.y);
        Draw(widget);
        ImGui::GetIO().AddMouseButtonEvent(button, true);
        Draw(widget);
        ImGui::GetIO().AddMouseButtonEvent(button, false);
        Draw(widget);
    }
};

using namespace cpptoolkit;

TEST_F(WidgetGalleryTests, SelectableUpdatesBeforeCallbackAndIgnoresPolling) {
    mvvm::ObservableProperty<bool> selected{nullptr, "Selected", false};
    int calls = 0;
    ui::SelectableWidget widget("Select", selected, [&](bool value) {
        EXPECT_EQ(selected.Get(), value);
        ++calls;
    });
    const auto position = Draw(widget);
    selected.Set(true);
    Draw(widget);
    EXPECT_EQ(calls, 0);
    Click(widget, position);
    EXPECT_FALSE(selected.Get());
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, ButtonExecutesCommandBeforeCallbackAndHonorsGuard) {
    int actions = 0, calls = 0;
    bool enabled = false;
    mvvm::Command command([&] { ++actions; }, [&] { return enabled; });
    ui::ButtonWidget widget("Action", command, [&] {
        EXPECT_EQ(actions, 1);
        ++calls;
    });
    const auto at = Draw(widget);
    Click(widget, at);
    EXPECT_EQ(actions, 0);
    EXPECT_EQ(calls, 0);
    enabled = true;
    Click(widget, at);
    EXPECT_EQ(actions, 1);
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, NotificationDismissesAfterUpdatingProperty) {
    mvvm::ObservableProperty<bool> open{nullptr, "Open", true};
    mvvm::ObservableProperty<std::string> message{nullptr, "Message", "Ready"};
    int calls = 0;
    ui::NotificationWidget widget("Info", message, open);
    widget.OnDismissed = [&] {
        EXPECT_FALSE(open.Get());
        ++calls;
    };
    const auto at = Draw(widget);
    Click(widget, at);
    EXPECT_FALSE(open.Get());
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, DateTimePickersValidateCivilValues) {
    using namespace std::chrono;
    const ui::Date leap{year{2000}, February, day{29}};
    EXPECT_NO_THROW((ui::DatePickerWidget("Leap", leap)));
    for (const ui::Date invalid : {ui::Date{year{1900}, February, day{29}},
                                  ui::Date{year{2024}, April, day{31}},
                                  ui::Date{year{0}, January, day{1}},
                                  ui::Date{year{10000}, January, day{1}}}) {
        EXPECT_THROW((ui::DatePickerWidget("Invalid", invalid)), std::invalid_argument);
        EXPECT_THROW((ui::DateTimePickerWidget("Invalid", ui::DateTime{invalid, seconds{0}})),
                     std::invalid_argument);
    }
    for (const auto invalid : {seconds{-1}, seconds{86400}, seconds::max()}) {
        EXPECT_THROW((ui::TimePickerWidget("Invalid", invalid)), std::invalid_argument);
        EXPECT_THROW((ui::DateTimePickerWidget("Invalid", ui::DateTime{leap, invalid})),
                     std::invalid_argument);
    }
    EXPECT_NO_THROW((ui::TimePickerWidget("Last second", seconds{86399})));
}

TEST_F(WidgetGalleryTests, TimePickerCommitsBeforeCallbackAndClampsWithoutCarrying) {
    using namespace std::chrono;
    mvvm::ObservableProperty<ui::TimeOfDay> time{nullptr, "Time", seconds{0}};
    int calls = 0;
    ui::TimePickerWidget picker("Time", time, [&](ui::TimeOfDay value) {
        EXPECT_EQ(time.Get(), value);
        ++calls;
    });
    const auto incrementSeconds = Draw(picker);
    Click(picker, incrementSeconds);
    EXPECT_EQ(time.Get(), seconds{1});
    EXPECT_EQ(calls, 1);
    time.Set(seconds{86398});
    Draw(picker);
    EXPECT_EQ(calls, 1);
    Click(picker, incrementSeconds);
    EXPECT_EQ(time.Get(), seconds{86399});
    EXPECT_EQ(calls, 2);
    Click(picker, incrementSeconds);
    EXPECT_EQ(time.Get(), seconds{86399});
    EXPECT_EQ(calls, 2);
    picker.readOnly = true;
    time.Set(seconds{0});
    Click(picker, incrementSeconds);
    EXPECT_EQ(time.Get(), seconds{0});
    EXPECT_EQ(calls, 2);
    picker.Visible = false;
    picker.Draw();
    EXPECT_EQ(calls, 2);
}

TEST_F(WidgetGalleryTests, DateTimePickerPreservesDateWhenEditingTimeAndSupportsLocalState) {
    using namespace std::chrono;
    const ui::Date date{year{2024}, February, day{29}};
    mvvm::ObservableProperty<ui::DateTime> value{nullptr, "DateTime", {date, seconds{42}}};
    int calls = 0;
    ui::DateTimePickerWidget picker("Date/time", value, [&](const ui::DateTime& changed) {
        EXPECT_EQ(value.Get(), changed);
        EXPECT_EQ(changed.date, date);
        ++calls;
    });
    const auto at = Draw(picker);
    Click(picker, at);
    EXPECT_EQ(value.Get().time, seconds{43});
    EXPECT_EQ(calls, 1);
    value.Set({date, seconds{10}});
    Draw(picker);
    EXPECT_EQ(calls, 1);
    picker.readOnly = true;
    Click(picker, at);
    EXPECT_EQ(value.Get().time, seconds{10});
    EXPECT_EQ(calls, 1);

    ui::DateTime received;
    ui::DateTimePickerWidget local("Local", ui::DateTime{date, seconds{0}},
                                  [&](const ui::DateTime& edited) { received = edited; });
    Click(local, Draw(local));
    EXPECT_EQ(received, (ui::DateTime{date, seconds{1}}));
    Click(local, Draw(local));
    EXPECT_EQ(received.time, seconds{2});
}

TEST_F(WidgetGalleryTests, CalendarSelectsLeapDayAndOnlyNotifiesActualUserChanges) {
    using namespace std::chrono;
    const ui::Date leap{year{2024}, February, day{29}};
    mvvm::ObservableProperty<ui::Date> date{nullptr, "Date", {year{2024}, February, day{28}}};
    int calls = 0;
    ui::DatePickerWidget picker("Calendar", date, [&](ui::Date changed) {
        EXPECT_EQ(date.Get(), changed);
        ++calls;
    });
    const auto open = Draw(picker);
    Click(picker, open);
    Draw(picker);
    auto& tables = ImGui::GetCurrentContext()->Tables;
    ASSERT_EQ(tables.GetAliveCount(), 1);
    const auto* calendar = tables.GetByIndex(0);
    ASSERT_EQ(calendar->ColumnsCount, 7);
    const auto& thursday = calendar->Columns[3];
    const ImVec2 day29(thursday.WorkMinX + 15,
                      calendar->RowPosY1 + ImGui::GetFontSize() / 2);
    Click(picker, day29);
    EXPECT_EQ(date.Get(), leap);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(ImGui::GetCurrentContext()->OpenPopupStack.Size, 0);
    Click(picker, open);
    Draw(picker);
    calendar = tables.GetByIndex(0);
    const ImVec2 sameDay(calendar->Columns[3].WorkMinX + 15,
                        calendar->RowPosY1 + ImGui::GetFontSize() / 2);
    Click(picker, sameDay);
    EXPECT_EQ(calls, 1);
    date.Set({year{2000}, February, day{29}});
    Draw(picker);
    EXPECT_EQ(calls, 1);
    picker.readOnly = true;
    Click(picker, open);
    EXPECT_EQ(ImGui::GetCurrentContext()->OpenPopupStack.Size, 0);
}

TEST_F(WidgetGalleryTests, CalendarsHandleMonthLengthsSixWeekRowsAndDateLimits) {
    using namespace std::chrono;
    const ui::Date lastDates[] = {
        {year{1900}, February, day{28}}, {year{2000}, February, day{29}},
        {year{2026}, August, day{31}}, {year{2026}, April, day{30}},
        {year{1}, January, day{31}}, {year{9999}, December, day{31}}};
    for (const auto lastDate : lastDates) {
        const ui::Date first{lastDate.year(), lastDate.month(), day{1}};
        ui::Date selected = first;
        ui::DatePickerWidget local("Month", first, [&](ui::Date value) { selected = value; });
        Click(local, Draw(local));
        Draw(local);
        auto& tables = ImGui::GetCurrentContext()->Tables;
        ASSERT_EQ(tables.GetAliveCount(), 1);
        const auto* table = tables.GetByIndex(0);
        const auto column = weekday{sys_days{lastDate}}.iso_encoding() - 1;
        const ImVec2 at(table->Columns[column].WorkMinX + 15,
                       table->RowPosY1 + ImGui::GetFontSize() / 2);
        Click(local, at);
        EXPECT_EQ(selected, lastDate);
        EXPECT_EQ(ImGui::GetCurrentContext()->OpenPopupStack.Size, 0);
    }
}

TEST_F(WidgetGalleryTests, CombinedCalendarEditsPreserveTimeAndBrowsingDoesNotCommit) {
    using namespace std::chrono;
    const ui::Date initial{year{2024}, December, day{1}};
    const auto time = seconds{45296};
    mvvm::ObservableProperty<ui::DateTime> value{nullptr, "Both", {initial, time}};
    int calls = 0;
    ui::DateTimePickerWidget picker("Combined", value, [&](const ui::DateTime& edited) {
        EXPECT_EQ(value.Get(), edited);
        EXPECT_EQ(edited.time, time);
        ++calls;
    });
    Draw(picker);
    // The date combo is the first input, immediately below the widget label.
    auto* host = ImGui::FindWindowByName("Gallery tests");
    ASSERT_NE(host, nullptr);
    const auto& style = ImGui::GetStyle();
    const ImVec2 open(host->WorkRect.Min.x + 50,
                      host->WorkRect.Min.y + ImGui::GetTextLineHeightWithSpacing() +
                      ImGui::GetFrameHeight() / 2);
    Click(picker, open);
    Draw(picker);
    auto& tables = ImGui::GetCurrentContext()->Tables;
    ASSERT_EQ(tables.GetAliveCount(), 1);
    auto* table = tables.GetByIndex(0);
    const auto popupPosition = table->InnerWindow->Pos;
    const float smallWidth = ImGui::CalcTextSize("<").x + 2 * style.FramePadding.x;
    const ImVec2 next(popupPosition.x + style.WindowPadding.x + smallWidth +
                      style.ItemSpacing.x + 80 + style.ItemInnerSpacing.x +
                      ImGui::CalcTextSize("Year").x + style.ItemSpacing.x + smallWidth / 2,
                      popupPosition.y + style.WindowPadding.y + ImGui::GetFrameHeight() / 2);
    Click(picker, next);
    EXPECT_EQ(value.Get(), (ui::DateTime{initial, time}));
    EXPECT_EQ(calls, 0);
    table = tables.GetByIndex(0);
    const ui::Date lastDate{year{2025}, January, day{31}};
    const auto column = weekday{sys_days{lastDate}}.iso_encoding() - 1;
    Click(picker, ImVec2(table->Columns[column].WorkMinX + 15,
                        table->RowPosY1 + ImGui::GetFontSize() / 2));
    EXPECT_EQ(value.Get(), (ui::DateTime{lastDate, time}));
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, PickersDiagnoseInvalidProgrammaticValuesWithoutRewritingThem) {
    using namespace std::chrono;
    mvvm::ObservableProperty<ui::Date> date{nullptr, "Date", ui::DateTime{}.date};
    mvvm::ObservableProperty<ui::TimeOfDay> time{nullptr, "Time", seconds{0}};
    mvvm::ObservableProperty<ui::DateTime> dateTime{nullptr, "DateTime", ui::DateTime{}};
    int calls = 0;
    ui::DatePickerWidget datePicker("Date", date, [&](ui::Date) { ++calls; });
    ui::TimePickerWidget timePicker("Time", time, [&](ui::TimeOfDay) { ++calls; });
    ui::DateTimePickerWidget combined("Both", dateTime, [&](const ui::DateTime&) { ++calls; });
    const ui::Date invalid{year{2025}, February, day{29}};
    date.Set(invalid);
    time.Set(seconds::max());
    dateTime.Set({invalid, seconds{-1}});
    Draw(datePicker);
    Draw(timePicker);
    Draw(combined);
    EXPECT_EQ(date.Get(), invalid);
    EXPECT_EQ(time.Get(), seconds::max());
    EXPECT_EQ(dateTime.Get(), (ui::DateTime{invalid, seconds{-1}}));
    EXPECT_EQ(calls, 0);
}

class CountingWidget : public ui::Widget {
public:
    void Build() override { ++Builds; }
    void Render() override { if (RenderEnabled) ++Renders; }
    void Draw() override { ++Draws; ImGui::TextUnformatted("Child"); }
    void Destroy() override { ++Destroys; }
    int Builds = 0, Renders = 0, Draws = 0, Destroys = 0;
};

TEST_F(WidgetGalleryTests, RenderTraversesHiddenInactiveAndSpecializedContainerChildren) {
    mvvm::ObservableObject model;
    ui::Panel root;
    root.SetViewModel(&model);
    root.Visible = false;
    auto& hidden = root.Add<CountingWidget>();
    hidden.Visible = false;
    auto& disabled = root.Add<CountingWidget>();
    disabled.RenderEnabled = false;
    mvvm::ObservableProperty<int> selected{nullptr, "Tab", 0};
    auto& tabs = root.Add<ui::TabBarWidget>("Tabs", selected);
    auto& first = tabs.AddTab("First").Add<CountingWidget>();
    auto& second = tabs.AddTab("Inactive").Add<CountingWidget>();
    mvvm::ObservableProperty<float> extent{nullptr, "Extent", 100};
    auto& splitter = root.Add<ui::SplitterWidget>("Split", extent);
    auto& left = splitter.First().Add<CountingWidget>();
    auto& right = splitter.Second().Add<CountingWidget>();
    root.Build();
    root.Render();
    EXPECT_EQ(hidden.Renders, 1);
    EXPECT_EQ(disabled.Renders, 0);
    EXPECT_EQ(first.Renders, 1);
    EXPECT_EQ(second.Renders, 1);
    EXPECT_EQ(left.Renders, 1);
    EXPECT_EQ(right.Renders, 1);
    EXPECT_EQ(second.GetViewModel<mvvm::ObservableObject>(), &model);
    tabs.RenderEnabled = false;
    splitter.First().RenderEnabled = false;
    root.Render();
    EXPECT_EQ(first.Renders, 1);
    EXPECT_EQ(second.Renders, 1);
    EXPECT_EQ(left.Renders, 1);
    EXPECT_EQ(right.Renders, 2);
    root.RenderEnabled = false;
    root.Render();
    EXPECT_EQ(hidden.Renders, 2);
    EXPECT_EQ(right.Renders, 2);
}

TEST_F(WidgetGalleryTests, RenderHookRunsBeforeChildrenWithoutStartingAnImGuiFrame) {
    std::vector<int> order;
    class RecordingWidget : public ui::Widget {
    public:
        RecordingWidget(std::vector<int>& order, int id) : order_(order), id_(id) {}
    protected:
        void OnRender() override {
            EXPECT_FALSE(ImGui::GetCurrentContext()->WithinFrameScope);
            order_.push_back(id_);
        }
    private:
        std::vector<int>& order_;
        int id_;
    };
    RecordingWidget root(order, 0);
    auto& nested = root.Add<RecordingWidget>(order, 1);
    nested.Add<RecordingWidget>(order, 2);
    root.Add<RecordingWidget>(order, 3);
    root.Render();
    EXPECT_EQ(order, (std::vector<int>{0, 1, 2, 3}));
}

TEST_F(WidgetGalleryTests, TeardownContinuesThroughContainersAfterChildFailure) {
    class FailingWidget : public ui::Widget {
    public:
        void Destroy() override { throw std::runtime_error("teardown failure"); }
    };
    class ReleasedWidget : public ui::Widget {
    public:
        explicit ReleasedWidget(int& count) : count_(count) {}
        void Destroy() override { ++count_; }
    private:
        int& count_;
    };
    int released = 0;
    ui::Panel root;
    root.Add<FailingWidget>();
    mvvm::ObservableProperty<int> selected{nullptr, "Tab", 0};
    auto& tabs = root.Add<ui::TabBarWidget>("Tabs", selected);
    tabs.AddTab("Fail").Add<FailingWidget>();
    tabs.AddTab("Release").Add<ReleasedWidget>(released);
    mvvm::ObservableProperty<float> size{nullptr, "Size", 100};
    auto& splitter = root.Add<ui::SplitterWidget>("Panes", size);
    splitter.First().Add<FailingWidget>();
    splitter.Second().Add<ReleasedWidget>(released);
    root.Add<ReleasedWidget>(released);
    EXPECT_THROW(root.Destroy(), std::runtime_error);
    EXPECT_EQ(released, 3);
    EXPECT_NO_THROW(root.Destroy());
    EXPECT_EQ(released, 3);
}

TEST_F(WidgetGalleryTests, RenderTextureRejectsInvalidConfigurationAndUnbuiltUse) {
    EXPECT_THROW((ui::RenderTextureWidget("Bad", 0, 10)), std::invalid_argument);
    EXPECT_THROW((ui::RenderTextureWidget("Bad", 10, -1)), std::invalid_argument);
    ui::RenderTextureWidget widget("Target", 32, 24);
    EXPECT_THROW(widget.SetDisplaySize(ImVec2(0, 10)), std::invalid_argument);
    EXPECT_THROW(widget.SetDisplaySize(ImVec2(10, std::numeric_limits<float>::infinity())),
                 std::invalid_argument);
    EXPECT_THROW(widget.GetRenderTexture(), std::logic_error);
    EXPECT_THROW(widget.Build(), std::logic_error);
    EXPECT_THROW(widget.Render(), std::logic_error);
    EXPECT_THROW(widget.Draw(), std::logic_error);
    widget.RenderEnabled = false;
    widget.Visible = false;
    EXPECT_NO_THROW(widget.Render());
    EXPECT_NO_THROW(widget.Draw());
    widget.Destroy();
    widget.Destroy();
}

TEST_F(WidgetGalleryTests, ContainersRespectVisibilityAndLifecycle) {
    ui::StackPanel stack(ui::Orientation::Horizontal);
    auto& visible = stack.Add<CountingWidget>();
    auto& hidden = stack.Add<CountingWidget>();
    hidden.Visible = false;
    stack.Build();
    Draw(stack);
    EXPECT_EQ(visible.Builds, 1);
    EXPECT_EQ(hidden.Builds, 1);
    EXPECT_EQ(visible.Draws, 1);
    EXPECT_EQ(hidden.Draws, 0);

    mvvm::ObservableProperty<float> extent{nullptr, "Extent", 200};
    ui::SplitterWidget splitter("Panes", extent);
    auto& first = splitter.First().Add<CountingWidget>();
    auto& second = splitter.Second().Add<CountingWidget>();
    splitter.Build();
    Draw(splitter);
    EXPECT_EQ(first.Builds, 1);
    EXPECT_EQ(second.Builds, 1);
    EXPECT_EQ(first.Draws, 1);
    EXPECT_EQ(second.Draws, 1);
}

TEST_F(WidgetGalleryTests, TabSelectionFromViewModelDoesNotCallHandler) {
    mvvm::ObservableProperty<int> selected{nullptr, "Selected", 0};
    ui::TabBarWidget tabs("Tabs", selected);
    auto& first = tabs.AddTab("First").Add<CountingWidget>();
    auto& second = tabs.AddTab("Second").Add<CountingWidget>();
    int calls = 0;
    tabs.OnSelectionChanged = [&](int) { ++calls; };
    tabs.Build();
    Draw(tabs);
    Draw(tabs);
    EXPECT_GT(first.Draws, 0);
    selected.Set(1);
    Draw(tabs);
    Draw(tabs);
    EXPECT_GT(second.Draws, 0);
    EXPECT_EQ(selected.Get(), 1);
    EXPECT_EQ(calls, 0);
}

TEST_F(WidgetGalleryTests, PopupProgrammaticOpenCloseAndUserDismissal) {
    mvvm::ObservableProperty<bool> open{nullptr, "Open", false};
    ui::PopupWidget popup("Popup", open);
    mvvm::Command close([&] { ImGui::CloseCurrentPopup(); });
    popup.Add<ui::ButtonWidget>("Close", close);
    int opens = 0, closes = 0;
    popup.OnOpened = [&] { EXPECT_TRUE(open.Get()); ++opens; };
    popup.OnClosed = [&] { EXPECT_FALSE(open.Get()); ++closes; };
    Draw(popup);
    open.Set(true);
    Draw(popup);
    Draw(popup);
    EXPECT_EQ(opens, 0);
    open.Set(false);
    Draw(popup);
    EXPECT_EQ(closes, 0);

    open.Set(true);
    Draw(popup);
    const auto at = Draw(popup);
    Click(popup, at);
    EXPECT_FALSE(open.Get());
    EXPECT_EQ(closes, 1);
}

TEST_F(WidgetGalleryTests, TreeRejectsDuplicateIdsAndTableHandlesRaggedRows) {
    mvvm::ObservableProperty<int> selected{nullptr, "Selected", -1};
    EXPECT_THROW((ui::TreeViewWidget("Tree", {{1, "Parent", {{1, "Duplicate", {}}}}}, selected)),
                 std::invalid_argument);
    mvvm::ObservableProperty<ui::TableWidget::Rows> rows{
        nullptr, "Rows", {{"A", "1"}, {}, {"B"}}};
    EXPECT_THROW((ui::TableWidget("Table", {}, rows, selected)), std::invalid_argument);
    ui::TableWidget table("Table", {"Name", "Value"}, rows, selected, true);
    int calls = 0;
    table.OnSelectionChanged = [&](int) { ++calls; };
    Draw(table);
    rows.Set({{"Updated", "2"}});
    selected.Set(0);
    Draw(table);
    EXPECT_EQ(calls, 0);
}

TEST_F(WidgetGalleryTests, TableFirstCellKeepsTextInsideItsColumnAndCommitsSelection) {
    mvvm::ObservableProperty<int> selected{nullptr, "Selected", -1};
    mvvm::ObservableProperty<ui::TableWidget::Rows> rows{
        nullptr, "Rows", {{"First##literal", "Value"}}};
    ui::TableWidget table("Table", {"Name", "Value"}, rows, selected);
    int calls = 0;
    table.OnSelectionChanged = [&](int row) {
        EXPECT_EQ(selected.Get(), row);
        ++calls;
    };
    Draw(table);
    Draw(table);
    auto& tables = ImGui::GetCurrentContext()->Tables;
    ASSERT_GT(tables.GetAliveCount(), 0);
    auto* state = tables.GetByIndex(0);
    ASSERT_NE(state, nullptr);
    ASSERT_EQ(state->ColumnsCount, 2);
    const auto& column = state->Columns[0];
    const ImVec2 at((column.MinX + column.MaxX) / 2, (state->RowPosY1 + state->RowPosY2) / 2);
    Click(table, at);
    EXPECT_EQ(selected.Get(), 0);
    EXPECT_EQ(calls, 1);
    const auto& window = *state->InnerWindow;
    EXPECT_LT(window.DC.CursorMaxPos.x, state->Columns[1].MaxX + 1);
}

TEST_F(WidgetGalleryTests, FeedbackAndLayoutsDrawWithBoundValues) {
    mvvm::ObservableProperty<float> progress{nullptr, "Progress", 0.5f};
    mvvm::ObservableProperty<bool> busy{nullptr, "Busy", true};
    mvvm::ObservableProperty<std::string> text{nullptr, "Text", "Badge"};
    mvvm::ObservableProperty<ImTextureID> texture{nullptr, "Texture", 1};
    ui::GridPanel grid("Grid", 2);
    grid.Add<ui::ProgressBarWidget>("Progress", progress);
    grid.Add<ui::SpinnerWidget>("Busy", busy);
    grid.Add<ui::BadgeWidget>(text);
    grid.Add<ui::ImageWidget>(texture, ImVec2(40, 40));
    auto& scroll = grid.Add<ui::ScrollPanel>("Scroll", ImVec2(0, 100));
    scroll.Add<ui::TextWidget>(text);
    Draw(grid);
    busy.Set(false);
    progress.Set(2);
    Draw(grid);
    EXPECT_THROW((ui::GridPanel("Invalid", 0)), std::invalid_argument);
    EXPECT_THROW((ui::ImageWidget(texture, ImVec2(-1, 0))), std::invalid_argument);
    progress.Set(std::numeric_limits<float>::quiet_NaN());
    Draw(grid);
}

TEST_F(WidgetGalleryTests, BreadcrumbUpdatesBeforeCallback) {
    mvvm::ObservableProperty<int> selected{nullptr, "Selected", 0};
    int calls = 0;
    ui::BreadcrumbWidget widget({"Root", "Device"}, selected, [&](int index) {
        EXPECT_EQ(selected.Get(), index);
        ++calls;
    });
    const auto at = Draw(widget);
    Click(widget, at);
    EXPECT_EQ(selected.Get(), 1);
    EXPECT_EQ(calls, 1);
    Click(widget, at);
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, ContextMenuUpdatesOpenStateForUserActions) {
    mvvm::ObservableProperty<bool> open{nullptr, "Open", false};
    mvvm::Command action([] {});
    ui::Panel root;
    root.Add<ui::ButtonWidget>("Right-click", action);
    auto& context = root.Add<ui::ContextMenuWidget>("Context", open);
    context.Add<ui::MenuItemWidget>("Action", action);
    int opens = 0, closes = 0;
    context.OnOpened = [&] { EXPECT_TRUE(open.Get()); ++opens; };
    context.OnClosed = [&] { EXPECT_FALSE(open.Get()); ++closes; };
    const auto at = Draw(root);
    Click(root, at, 1);
    Draw(root);
    EXPECT_TRUE(open.Get());
    EXPECT_EQ(opens, 1);
    Click(root, ImVec2(850, 750));
    EXPECT_FALSE(open.Get());
    EXPECT_EQ(closes, 1);
}

TEST_F(WidgetGalleryTests, TabClickUpdatesPropertyBeforeCallback) {
    mvvm::ObservableProperty<int> selected{nullptr, "Selected", 0};
    ui::TabBarWidget tabs("Tabs", selected);
    tabs.AddTab("First").Add<CountingWidget>();
    tabs.AddTab("Second").Add<CountingWidget>();
    int calls = 0;
    tabs.OnSelectionChanged = [&](int index) {
        EXPECT_EQ(selected.Get(), index);
        ++calls;
    };
    Draw(tabs);
    Draw(tabs);
    auto& pool = ImGui::GetCurrentContext()->TabBars;
    ASSERT_GT(pool.GetAliveCount(), 0);
    ImGuiTabBar* bar = pool.GetByIndex(0);
    ASSERT_NE(bar, nullptr);
    ASSERT_EQ(bar->Tabs.Size, 2);
    const auto& tab = bar->Tabs[1];
    const ImVec2 at(bar->BarRect.Min.x + tab.Offset + tab.Width / 2,
                    (bar->BarRect.Min.y + bar->BarRect.Max.y) / 2);
    Click(tabs, at);
    Draw(tabs);
    EXPECT_EQ(selected.Get(), 1);
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, ModalCanBeClosedThroughCommand) {
    class RecordingButton : public ui::ButtonWidget {
    public:
        using ui::ButtonWidget::ButtonWidget;
        void Draw() override {
            ui::ButtonWidget::Draw();
            const auto min = ImGui::GetItemRectMin();
            const auto max = ImGui::GetItemRectMax();
            Center = ImVec2((min.x + max.x) / 2, (min.y + max.y) / 2);
        }
        ImVec2 Center;
    };
    mvvm::ObservableProperty<bool> open{nullptr, "Open", true};
    mvvm::Command close([&] { open.Set(false); });
    ui::DialogWidget dialog("Dialog", open);
    auto& button = dialog.Add<RecordingButton>("Confirm", close);
    int calls = 0;
    dialog.OnClosed = [&] { EXPECT_FALSE(open.Get()); ++calls; };
    Draw(dialog);
    Draw(dialog);
    Draw(dialog);
    Click(dialog, button.Center);
    EXPECT_FALSE(open.Get());
    EXPECT_EQ(calls, 1);
}

TEST_F(WidgetGalleryTests, TestUIBuildsAndDrawsEveryGalleryTab) {
    TestUIState state;
    ui::Panel root;
    BuildTestUIWidgets(root, state);
    root.Build();
    for (int tab = 0; tab < 6; ++tab) {
        state.SelectedTab.Set(tab);
        state.Tick(1.0f / 60);
        Draw(root);
        Draw(root);
        Draw(root);
        EXPECT_EQ(state.SelectedTab.Get(), tab);
    }
    state.OpenDialog.Execute();
    Draw(root);
    EXPECT_TRUE(state.DialogOpen.Get());
    state.CloseDialog.Execute();
    Draw(root);
    EXPECT_FALSE(state.DialogOpen.Get());
    root.Destroy();
}

TEST_F(WidgetGalleryTests, SliderCommitsUserChangesBeforeCallbackAndValidatesRange) {
    mvvm::ObservableProperty<float> value{nullptr, "Value", 0};
    int calls = 0;
    ui::SliderFloatWidget slider("Slider", value, 0, 1, [&](float changed) {
        EXPECT_EQ(value.Get(), changed);
        ++calls;
    });
    const auto at = Draw(slider);
    value.Set(0.1f);
    Draw(slider);
    EXPECT_EQ(calls, 0);
    Click(slider, at);
    EXPECT_GT(value.Get(), 0.1f);
    EXPECT_GT(calls, 0);
    EXPECT_THROW((ui::SliderFloatWidget("Invalid", value, 1, 0)), std::invalid_argument);
    EXPECT_THROW((ui::SliderFloatWidget("Invalid", value, 0,
                                       std::numeric_limits<float>::infinity())),
                 std::invalid_argument);
}

TEST_F(WidgetGalleryTests, AmbientContextsCascadeThroughSpecializedContainers) {
    mvvm::ObservableObject model, ownModel;
    ui::Application application;
    ui::Panel root;
    root.SetViewModel(&model);
    root.SetApplication(&application);
    auto& nested = root.Add<ui::Panel>();
    auto& child = nested.Add<CountingWidget>();
    EXPECT_EQ(child.GetViewModel<mvvm::ObservableObject>(), &model);
    EXPECT_EQ(child.GetApplication(), &application);

    mvvm::ObservableProperty<int> selected{nullptr, "Tab", 0};
    auto& tabs = root.Add<ui::TabBarWidget>("Tabs", selected);
    auto& tabChild = tabs.AddTab("A").Add<CountingWidget>();
    mvvm::ObservableProperty<float> size{nullptr, "Size", 100};
    auto& splitter = root.Add<ui::SplitterWidget>("Splitter", size);
    auto& paneChild = splitter.First().Add<CountingWidget>();
    paneChild.SetViewModel(&ownModel);
    root.Build();
    EXPECT_EQ(tabChild.GetViewModel<mvvm::ObservableObject>(), &model);
    EXPECT_EQ(tabChild.GetApplication(), &application);
    EXPECT_EQ(paneChild.GetViewModel<mvvm::ObservableObject>(), &ownModel);
    EXPECT_EQ(paneChild.GetApplication(), &application);
}

TEST_F(WidgetGalleryTests, OnBuildHookRecursesAndPreBuildReceivesAmbientContext) {
    class HookWidget : public ui::Widget {
    public:
        bool Prepared = false;
        int Builds = 0;
        void PreBuild() override { Prepared = HasViewModel(); }
    protected:
        void OnBuild() override { ++Builds; Add<CountingWidget>(); }
    };
    mvvm::ObservableObject model;
    ui::Panel root;
    root.SetViewModel(&model);
    auto& widget = root.Add<HookWidget>();
    EXPECT_TRUE(widget.Prepared);
    root.Build();
    EXPECT_EQ(widget.Builds, 1);
    Draw(root);
}

TEST_F(WidgetGalleryTests, OptionalBindingsAndCallbackButtonsRemainUsable) {
    ui::Panel panel;
    panel.Add<ui::TextWidget>("Static text");
    panel.Add<ui::PlotLineWidget>("Unbound plot");
    auto& slider = panel.Add<ui::SliderFloatWidget>("Unbound slider", 0, 1, 0.5f);
    Draw(panel);
    int clicks = 0;
    ui::ButtonWidget button("Callback button", [&] { ++clicks; });
    Click(button, Draw(button));
    EXPECT_EQ(clicks, 1);
    int edits = 0;
    slider.OnValueChanged = [&](float) { ++edits; };
    Click(slider, Draw(slider));
    EXPECT_GT(edits, 0);
}

TEST_F(WidgetGalleryTests, TitledPanelClosesAndReopensWithoutChangingInlineContainers) {
    ui::Panel window("Titled panel");
    auto& child = window.Add<CountingWidget>();
    window.SetFlags(ImGuiWindowFlags_NoCollapse);
    EXPECT_TRUE(window.HasFlag(ImGuiWindowFlags_NoCollapse));
    Draw(window);
    EXPECT_GT(child.Draws, 0);
    window.SetOpen(false);
    const int before = child.Draws;
    Draw(window);
    EXPECT_EQ(child.Draws, before);
    window.SetOpen(true);
    Draw(window);
    EXPECT_GT(child.Draws, before);
    window.ClearFlags(ImGuiWindowFlags_NoCollapse);
    EXPECT_FALSE(window.HasFlag(ImGuiWindowFlags_NoCollapse));
}

TEST(ApplicationMergeTests, SingletonFactoryAndLegacyAliasShareOneInstance) {
    class Root : public ui::Widget {};
    class OtherRoot : public ui::Widget {};
    EXPECT_THROW(ui::Application::Instance(), std::logic_error);
    auto& application = ui::Application::Create<Root>();
    EXPECT_EQ(&application, &ui::Application::Instance());
    EXPECT_EQ(&application, &ui::Application::GetInstance<Root>());
    EXPECT_THROW(ui::Application::Create<OtherRoot>(), std::logic_error);
    application.SetConfigFlags(FLAG_WINDOW_RESIZABLE).SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    EXPECT_EQ(application.GetConfigFlags(), FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    application.ClearConfigFlags(FLAG_WINDOW_HIGHDPI);
    EXPECT_EQ(application.GetConfigFlags(), FLAG_WINDOW_RESIZABLE);
}

TEST(ApplicationMergeTests, AsyncQueueAndRenderDispatchExecuteOnSeparateThreads) {
    const auto callingThread = std::this_thread::get_id();
    std::promise<std::thread::id> completed;
    auto future = completed.get_future();
    ui::Application::RunAsync([&] { completed.set_value(std::this_thread::get_id()); });
    ASSERT_EQ(future.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    EXPECT_NE(future.get(), callingThread);
    bool called = false;
    ui::Application::Dispatch([&] {
        EXPECT_EQ(std::this_thread::get_id(), callingThread);
        called = true;
    });
    EXPECT_FALSE(called);
    mvvm::Dispatcher::Main().ProcessPending();
    EXPECT_TRUE(called);
}

} // namespace
