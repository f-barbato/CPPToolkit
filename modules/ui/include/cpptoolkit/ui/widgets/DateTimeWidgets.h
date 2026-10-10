#pragma once

/**
 * @file DateTimeWidgets.h
 * @brief Gregorian calendar and 24-hour civil-time pickers.
 *
 * Values have no time zone, UTC offset or daylight-saving conversion. Borrowed
 * properties must outlive their widgets. Draw() polls them on the render thread
 * in an active ImGui frame/window. Actual user edits commit before callbacks;
 * programmatic Set() calls are silent. Invalid initial values throw and invalid
 * later property values display a diagnostic without being rewritten.
 */

#include <chrono>
#include <functional>
#include <string>

#include <cpptoolkit/mvvm/ObservableProperty.h>
#include <cpptoolkit/ui/Widget.h>

namespace cpptoolkit::ui {

/** @brief Gregorian civil date; pickers support valid dates in years 1 through 9999. */
using Date = std::chrono::year_month_day;
/** @brief Seconds since midnight, in [0, 86399]; leap seconds are not supported. */
using TimeOfDay = std::chrono::seconds;

/** @brief Civil date and time, not an absolute timestamp. */
struct DateTime {
    /** @brief Gregorian date, defaulting to 1970-01-01. */
    Date date{std::chrono::year{1970}, std::chrono::January, std::chrono::day{1}};
    /** @brief Seconds since midnight, defaulting to 00:00:00. */
    TimeOfDay time{0};
    /** @brief Compares both civil components.
     * @param other Value to compare.
     * @return Whether both date and time are equal.
     */
    bool operator==(const DateTime& other) const = default;
};

/** @brief Drop-down calendar with Monday-first weeks and ISO date preview. */
class CPPTOOLKIT_UI_EXPORT DatePickerWidget : public Widget {
public:
    /** @brief Receives the committed user-selected date. */
    using DateChangedHandler = std::function<void(Date)>;
    /** @brief Creates a two-way bound calendar.
     * @param label ImGui label and identifier.
     * @param bound Borrowed date property, required to outlive this widget.
     * @param onDateChanged Optional render-thread callback after commit.
     * @throws std::invalid_argument If the initial date is invalid or outside years 1-9999.
     */
    DatePickerWidget(std::string label, mvvm::ObservableProperty<Date>& bound,
                     DateChangedHandler onDateChanged = {});
    /** @brief Creates a calendar with local state and an optional callback.
     * @param label ImGui label and identifier.
     * @param initial Initial local date; defaults to 1970-01-01.
     * @param onDateChanged Optional render-thread callback after a user edit.
     * @throws std::invalid_argument If initial is invalid or outside years 1-9999.
     */
    explicit DatePickerWidget(std::string label, Date initial = DateTime{}.date,
                              DateChangedHandler onDateChanged = {});
    /** @brief Draws when Visible; selecting a day closes the calendar. */
    void Draw() override;
    /** @brief User-edit callback; selecting the existing date does not notify. */
    DateChangedHandler OnDateChanged;
    /** @brief Disables user interaction while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<Date>* bound_ = nullptr;
    Date value_;
    std::chrono::year_month month_;
};

/** @brief Hours/minutes/seconds editor using 24-hour time and bounded step buttons.
 *
 * Each component clamps independently; stepping past 59 seconds or minutes
 * does not carry into the next component, and hours do not wrap past 23.
 */
class CPPTOOLKIT_UI_EXPORT TimePickerWidget : public Widget {
public:
    /** @brief Receives the committed user-edited time. */
    using TimeChangedHandler = std::function<void(TimeOfDay)>;
    /** @brief Creates a two-way bound time editor.
     * @param label ImGui label and identifier.
     * @param bound Borrowed time property, required to outlive this widget.
     * @param onTimeChanged Optional render-thread callback after commit.
     * @throws std::invalid_argument If the initial value is outside [0, 86399] seconds.
     */
    TimePickerWidget(std::string label, mvvm::ObservableProperty<TimeOfDay>& bound,
                     TimeChangedHandler onTimeChanged = {});
    /** @brief Creates a time editor with local state and an optional callback.
     * @param label ImGui label and identifier.
     * @param initial Initial local time; defaults to midnight.
     * @param onTimeChanged Optional render-thread callback after a user edit.
     * @throws std::invalid_argument If initial is outside [0, 86399] seconds.
     */
    explicit TimePickerWidget(std::string label, TimeOfDay initial = TimeOfDay{0},
                              TimeChangedHandler onTimeChanged = {});
    /** @brief Draws when Visible and commits actual user changes. */
    void Draw() override;
    /** @brief User-edit callback; programmatic updates are silent. */
    TimeChangedHandler OnTimeChanged;
    /** @brief Disables all user edits while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<TimeOfDay>* bound_ = nullptr;
    TimeOfDay value_;
};

/** @brief Calendar and time editor bound atomically to one civil DateTime value.
 *
 * Uses the same date range and independent time-component clamping as the
 * standalone pickers. There is no conversion to a system clock timestamp.
 */
class CPPTOOLKIT_UI_EXPORT DateTimePickerWidget : public Widget {
public:
    /** @brief Receives the committed date/time; reference lasts only for the call. */
    using DateTimeChangedHandler = std::function<void(const DateTime&)>;
    /** @brief Creates a two-way bound date/time editor.
     * @param label ImGui label and identifier.
     * @param bound Borrowed property, required to outlive this widget.
     * @param onDateTimeChanged Optional render-thread callback after commit.
     * @throws std::invalid_argument If either initial component is invalid.
     */
    DateTimePickerWidget(std::string label, mvvm::ObservableProperty<DateTime>& bound,
                         DateTimeChangedHandler onDateTimeChanged = {});
    /** @brief Creates a date/time editor with local state and an optional callback.
     * @param label ImGui label and identifier.
     * @param initial Initial civil value; defaults to 1970-01-01 00:00:00.
     * @param onDateTimeChanged Optional render-thread callback after a user edit.
     * @throws std::invalid_argument If either initial component is invalid.
     */
    explicit DateTimePickerWidget(std::string label, DateTime initial = {},
                                  DateTimeChangedHandler onDateTimeChanged = {});
    /** @brief Draws when Visible and commits both components before one callback. */
    void Draw() override;
    /** @brief User-edit callback; programmatic updates are silent. */
    DateTimeChangedHandler OnDateTimeChanged;
    /** @brief Disables calendar and time edits; property polling continues. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<DateTime>* bound_ = nullptr;
    DateTime value_;
    std::chrono::year_month month_;
};

} // namespace cpptoolkit::ui
