#include <cpptoolkit/ui/widgets/DateTimeWidgets.h>

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <utility>

#include <imgui.h>

namespace cpptoolkit::ui {
namespace {

bool Valid(Date date) {
    return date.ok() && int(date.year()) >= 1 && int(date.year()) <= 9999;
}

bool Valid(TimeOfDay time) {
    return time >= TimeOfDay{0} && time < std::chrono::days{1};
}

bool Valid(const DateTime& value) {
    return Valid(value.date) && Valid(value.time);
}

template <typename T>
void Validate(const T& value) {
    if (!Valid(value)) throw std::invalid_argument("Invalid civil date/time picker value");
}

template <typename T, typename Handler>
void Commit(mvvm::ObservableProperty<T>* bound, T& local, const T& previous,
            const T& value, const Handler& handler) {
    if (value == previous) return;
    local = value;
    if (bound) bound->Set(value);
    if (handler) handler(value);
}

bool DrawCalendar(const char* label, Date& value, std::chrono::year_month& displayed) {
    using namespace std::chrono;
    char preview[11];
    std::snprintf(preview, sizeof(preview), "%04d-%02u-%02u",
                  int(value.year()), unsigned(value.month()), unsigned(value.day()));
    bool changed = false;
    if (ImGui::BeginCombo(label, preview, ImGuiComboFlags_HeightLargest)) {
        if (ImGui::IsWindowAppearing()) displayed = value.year() / value.month();
        ImGui::BeginDisabled(displayed == year{1} / January);
        if (ImGui::SmallButton("<")) displayed -= months{1};
        ImGui::EndDisabled();
        ImGui::SameLine();
        int selectedYear = int(displayed.year());
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputInt("Year", &selectedYear, 0, 0)) {
            displayed = year{std::clamp(selectedYear, 1, 9999)} / displayed.month();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(displayed == year{9999} / December);
        if (ImGui::SmallButton(">")) displayed += months{1};
        ImGui::EndDisabled();

        constexpr const char* names[] = {"January", "February", "March", "April",
            "May", "June", "July", "August", "September", "October", "November", "December"};
        int selectedMonth = static_cast<int>(unsigned(displayed.month())) - 1;
        if (ImGui::Combo("Month", &selectedMonth, names, 12)) {
            displayed = displayed.year() / month{static_cast<unsigned>(selectedMonth + 1)};
        }
        constexpr const char* weekdays[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
        if (ImGui::BeginTable("Calendar", 7, ImGuiTableFlags_SizingFixedSame)) {
            for (auto weekday : weekdays) ImGui::TableSetupColumn(weekday, ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableHeadersRow();
            const Date first{displayed / day{1}};
            const unsigned offset = weekday{sys_days{first}}.iso_encoding() - 1;
            const unsigned count = unsigned(year_month_day_last{displayed / last}.day());
            const unsigned cells = ((offset + count + 6) / 7) * 7;
            for (unsigned cell = 0; cell < cells; ++cell) {
                ImGui::TableNextColumn();
                if (cell < offset || cell >= offset + count) continue;
                const unsigned number = cell - offset + 1;
                const Date candidate{displayed / day{number}};
                char text[3];
                std::snprintf(text, sizeof(text), "%u", number);
                if (ImGui::Selectable(text, candidate == value, 0, ImVec2(30, 0))) {
                    value = candidate;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool DrawTime(TimeOfDay& value) {
    const auto seconds = value.count();
    int components[] = {static_cast<int>(seconds / 3600),
                        static_cast<int>((seconds / 60) % 60),
                        static_cast<int>(seconds % 60)};
    constexpr const char* labels[] = {"Hour", "Minute", "Second"};
    bool changed = false;
    for (int i = 0; i < 3; ++i) {
        if (i) ImGui::SameLine();
        ImGui::PushID(i);
        ImGui::SetNextItemWidth(75);
        const int limit = i == 0 ? 23 : 59;
        changed |= ImGui::DragInt(labels[i], &components[i], 1, 0, limit, "%02d",
                                 ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) { --components[i]; changed = true; }
        ImGui::SameLine();
        if (ImGui::SmallButton("+")) { ++components[i]; changed = true; }
        components[i] = std::clamp(components[i], 0, limit);
        ImGui::PopID();
    }
    if (changed) value = TimeOfDay{components[0] * 3600 + components[1] * 60 + components[2]};
    return changed;
}

} // namespace

DatePickerWidget::DatePickerWidget(std::string label, Date initial, DateChangedHandler handler)
    : OnDateChanged(std::move(handler)), label_(std::move(label)), value_(initial),
      month_(initial.year() / initial.month()) {
    Validate(initial);
}

DatePickerWidget::DatePickerWidget(std::string label, mvvm::ObservableProperty<Date>& bound,
                                   DateChangedHandler handler)
    : DatePickerWidget(std::move(label), bound.Get(), std::move(handler)) {
    bound_ = &bound;
}

void DatePickerWidget::Draw() {
    if (!Visible) return;
    const auto previous = bound_ ? bound_->Get() : value_;
    if (!Valid(previous)) {
        ImGui::Text("%s: invalid date (expected years 1-9999)", label_.c_str());
        return;
    }
    auto value = previous;
    ImGui::PushID(this);
    ImGui::BeginDisabled(readOnly);
    const bool changed = DrawCalendar(label_.c_str(), value, month_);
    ImGui::EndDisabled();
    ImGui::PopID();
    if (changed && !readOnly) Commit(bound_, value_, previous, value, OnDateChanged);
}

TimePickerWidget::TimePickerWidget(std::string label, TimeOfDay initial, TimeChangedHandler handler)
    : OnTimeChanged(std::move(handler)), label_(std::move(label)), value_(initial) {
    Validate(initial);
}

TimePickerWidget::TimePickerWidget(std::string label, mvvm::ObservableProperty<TimeOfDay>& bound,
                                   TimeChangedHandler handler)
    : TimePickerWidget(std::move(label), bound.Get(), std::move(handler)) {
    bound_ = &bound;
}

void TimePickerWidget::Draw() {
    if (!Visible) return;
    const auto previous = bound_ ? bound_->Get() : value_;
    if (!Valid(previous)) {
        ImGui::Text("%s: invalid time (expected 00:00:00-23:59:59)", label_.c_str());
        return;
    }
    auto value = previous;
    ImGui::PushID(this);
    ImGui::TextUnformatted(label_.c_str());
    ImGui::BeginDisabled(readOnly);
    const bool changed = DrawTime(value);
    ImGui::EndDisabled();
    ImGui::PopID();
    if (changed && !readOnly) Commit(bound_, value_, previous, value, OnTimeChanged);
}

DateTimePickerWidget::DateTimePickerWidget(std::string label, DateTime initial,
                                           DateTimeChangedHandler handler)
    : OnDateTimeChanged(std::move(handler)), label_(std::move(label)), value_(initial),
      month_(initial.date.year() / initial.date.month()) {
    Validate(initial);
}

DateTimePickerWidget::DateTimePickerWidget(std::string label,
                                           mvvm::ObservableProperty<DateTime>& bound,
                                           DateTimeChangedHandler handler)
    : DateTimePickerWidget(std::move(label), bound.Get(), std::move(handler)) {
    bound_ = &bound;
}

void DateTimePickerWidget::Draw() {
    if (!Visible) return;
    const auto previous = bound_ ? bound_->Get() : value_;
    if (!Valid(previous)) {
        ImGui::Text("%s: invalid civil date/time", label_.c_str());
        return;
    }
    auto value = previous;
    ImGui::PushID(this);
    ImGui::TextUnformatted(label_.c_str());
    ImGui::BeginDisabled(readOnly);
    bool changed = DrawCalendar("Date", value.date, month_);
    changed |= DrawTime(value.time);
    ImGui::EndDisabled();
    ImGui::PopID();
    if (changed && !readOnly) Commit(bound_, value_, previous, value, OnDateTimeChanged);
}

} // namespace cpptoolkit::ui
