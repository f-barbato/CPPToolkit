#pragma once

/**
 * @file InputWidgets.h
 * @brief Retained input and selection controls with two-way MVVM bindings.
 *
 * Bound properties are borrowed and must outlive their widgets. Draw() must run
 * on the render thread with a current ImGui context and an active frame/window.
 * Each visible control polls its property every frame. Accepted user changes
 * commit the property before invoking the optional widget callback synchronously
 * on the render thread. Programmatic property Set() calls never invoke these
 * widget callbacks. Callback argument references are valid only during the call.
 */

#include <array>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#include <cpptoolkit/mvvm/ObservableProperty.h>
#include <cpptoolkit/ui/Widget.h>

namespace cpptoolkit::ui {

/** @brief Two-way bound Boolean checkbox; the bound property must outlive it. */
class CPPTOOLKIT_UI_EXPORT CheckBoxWidget : public Widget {
public:
    /** @brief Receives the new checked state after a user change is committed. */
    using CheckedChangedHandler = std::function<void(bool)>;
    /**
     * @brief Creates a checkbox with an optional user-event handler.
     * @param label ImGui label and identifier.
     * @param bound Borrowed Boolean property, required to outlive this widget.
     * @param onCheckedChanged Optional render-thread callback after property commit.
     */
    CheckBoxWidget(std::string label, mvvm::ObservableProperty<bool>& bound,
                   CheckedChangedHandler onCheckedChanged = {});
    /** @brief Draws when Visible, polls the property, and commits user toggles. */
    void Draw() override;
    /** @brief Assignable user-change callback; programmatic Set() is silent. */
    CheckedChangedHandler OnCheckedChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<bool>& bound_;
};

/**
 * @brief Radio option sharing a borrowed integer selection property.
 *
 * Nonnegative option values identify choices; -1 denotes no selection. A single
 * radio widget cannot validate the upper limit of an entire group. Clicking an
 * already selected option does not notify. The property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT RadioButtonWidget : public Widget {
public:
    /** @brief Receives the newly selected option after its property is committed. */
    using SelectionChangedHandler = std::function<void(int)>;
    /**
     * @brief Creates one option in a radio group.
     * @param label ImGui label and identifier.
     * @param bound Borrowed group selection property, required to outlive this widget.
     * @param option Nonnegative value written when the user selects this option.
     * @param onSelectionChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If option is negative or initial selection is below -1.
     */
    RadioButtonWidget(std::string label, mvvm::ObservableProperty<int>& bound, int option,
                      SelectionChangedHandler onSelectionChanged = {});
    /** @brief Draws when Visible and commits a user selection if it actually changes. */
    void Draw() override;
    /** @brief Assignable user-selection callback; programmatic Set() is silent. */
    SelectionChangedHandler OnSelectionChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<int>& bound_;
    int option_;
};

/**
 * @brief Drop-down selection over an owned, immutable list of labels.
 *
 * Selection is a zero-based index, with -1 meaning no selection. Empty choices
 * require an initial selection of -1. Invalid selections set after construction
 * show an empty preview without changing the property. Choices are copied or
 * moved into the widget and cannot subsequently be replaced. The borrowed
 * selection property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT ComboBoxWidget : public Widget {
public:
    /** @brief Receives the new zero-based selection after property commit. */
    using SelectionChangedHandler = std::function<void(int)>;
    /**
     * @brief Creates a combo box and validates its initial selection.
     * @param label ImGui label and identifier.
     * @param bound Borrowed selection property, required to outlive this widget.
     * @param choices Owned static choices; duplicate labels are supported.
     * @param onSelectionChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If initial selection is not -1 or a valid index,
     *         or the choices count exceeds the range of int.
     */
    ComboBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                   std::vector<std::string> choices,
                   SelectionChangedHandler onSelectionChanged = {});
    /** @brief Draws when Visible and commits actual user selection changes. */
    void Draw() override;
    /** @brief Assignable user-selection callback; programmatic Set() is silent. */
    SelectionChangedHandler OnSelectionChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<int>& bound_;
    std::vector<std::string> choices_;
};

/**
 * @brief List selection over an owned, immutable list of labels.
 *
 * Selection is a zero-based index, with -1 meaning no selection. Empty choices
 * require an initial selection of -1. Invalid selections set after construction
 * safely display no selected row without changing the property. The borrowed
 * selection property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT ListBoxWidget : public Widget {
public:
    /** @brief Receives the new zero-based selection after property commit. */
    using SelectionChangedHandler = std::function<void(int)>;
    /**
     * @brief Creates a list box and validates its initial selection.
     * @param label ImGui label and identifier.
     * @param bound Borrowed selection property, required to outlive this widget.
     * @param choices Owned static choices, not replaceable after construction;
     *                duplicate labels are supported.
     * @param onSelectionChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If initial selection is not -1 or a valid index,
     *         or the choices count exceeds the range of int.
     */
    ListBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                  std::vector<std::string> choices,
                  SelectionChangedHandler onSelectionChanged = {});
    /** @brief Draws when Visible and commits actual user selection changes. */
    void Draw() override;
    /** @brief Assignable user-selection callback; programmatic Set() is silent. */
    SelectionChangedHandler OnSelectionChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<int>& bound_;
    std::vector<std::string> choices_;
};

/**
 * @brief Integer text input with optional ImGui step buttons.
 *
 * No numeric bounds are imposed. Accepted edits follow ImGui's scalar-input
 * commit behavior. The borrowed property must outlive this widget.
 */
class CPPTOOLKIT_UI_EXPORT InputIntWidget : public Widget {
public:
    /** @brief Receives the new integer after an accepted user edit is committed. */
    using ValueChangedHandler = std::function<void(int)>;
    /**
     * @brief Creates integer input with configurable step amounts.
     * @param label ImGui label and identifier.
     * @param bound Borrowed integer property, required to outlive this widget.
     * @param step Nonnegative normal step; zero omits the step buttons.
     * @param stepFast Nonnegative step used with ImGui's fast-step modifier.
     * @param onValueChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If either step is negative.
     */
    InputIntWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                   int step = 1, int stepFast = 100, ValueChangedHandler onValueChanged = {});
    /** @brief Draws when Visible and commits accepted, actual user value changes. */
    void Draw() override;
    /** @brief Assignable user-value callback; programmatic Set() is silent. */
    ValueChangedHandler OnValueChanged;
    /** @brief Prevents user edits and stepping while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<int>& bound_;
    int step_;
    int stepFast_;
};

/**
 * @brief Floating-point text input with optional ImGui step buttons.
 *
 * No numeric bounds are imposed. Display uses three fractional digits. Accepted
 * edits follow ImGui's scalar-input commit behavior. The property must outlive it.
 */
class CPPTOOLKIT_UI_EXPORT InputFloatWidget : public Widget {
public:
    /** @brief Receives the new value after an accepted user edit is committed. */
    using ValueChangedHandler = std::function<void(float)>;
    /**
     * @brief Creates floating-point input with configurable step amounts.
     * @param label ImGui label and identifier.
     * @param bound Borrowed floating-point property, required to outlive this widget.
     * @param step Finite, nonnegative normal step; zero omits the step buttons.
     * @param stepFast Finite, nonnegative step for ImGui's fast-step modifier.
     * @param onValueChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If either step is negative or nonfinite.
     */
    InputFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                     float step = 0.0f, float stepFast = 0.0f,
                     ValueChangedHandler onValueChanged = {});
    /** @brief Draws when Visible and commits accepted, actual user value changes. */
    void Draw() override;
    /** @brief Assignable user-value callback; programmatic Set() is silent. */
    ValueChangedHandler OnValueChanged;
    /** @brief Prevents user edits and stepping while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    float step_;
    float stepFast_;
};

/**
 * @brief Mouse-draggable floating-point input with inclusive user-edit bounds.
 *
 * Accepted user changes are clamped to [min, max]; merely drawing does not clamp
 * programmatic property values. Equal bounds fix user edits to that value.
 * The borrowed property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT DragFloatWidget : public Widget {
public:
    /** @brief Receives the clamped value after an actual user change is committed. */
    using ValueChangedHandler = std::function<void(float)>;
    /**
     * @brief Creates bounded draggable input.
     * @param label ImGui label and identifier.
     * @param bound Borrowed floating-point property, required to outlive this widget.
     * @param speed Finite, positive drag sensitivity.
     * @param min Finite inclusive lower user-edit bound.
     * @param max Finite inclusive upper user-edit bound, at least min.
     * @param onValueChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If speed is nonfinite or not positive,
     *         a bound is nonfinite, or min exceeds max.
     */
    DragFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                    float speed = 1.0f, float min = std::numeric_limits<float>::lowest(),
                    float max = std::numeric_limits<float>::max(),
                    ValueChangedHandler onValueChanged = {});
    /** @brief Draws when Visible and commits actual clamped user changes. */
    void Draw() override;
    /** @brief Assignable user-value callback; programmatic Set() is silent. */
    ValueChangedHandler OnValueChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<float>& bound_;
    float speed_;
    float min_;
    float max_;
};

/**
 * @brief Bounded integer text input with decrement and increment buttons.
 *
 * Accepted user edits and button steps clamp to [min, max]. Step arithmetic is
 * overflow-safe; merely drawing does not clamp programmatic property values.
 * The borrowed property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT SpinBoxWidget : public Widget {
public:
    /** @brief Receives the clamped integer after an actual user change is committed. */
    using ValueChangedHandler = std::function<void(int)>;
    /**
     * @brief Creates a bounded integer spin box.
     * @param label ImGui label and identifier.
     * @param bound Borrowed integer property, required to outlive this widget.
     * @param step Positive amount added or subtracted by the buttons.
     * @param min Inclusive lower user-edit bound.
     * @param max Inclusive upper user-edit bound, at least min.
     * @param onValueChanged Optional render-thread callback after property commit.
     * @throws std::invalid_argument If step is not positive or min exceeds max.
     */
    SpinBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                  int step, int min, int max, ValueChangedHandler onValueChanged = {});
    /** @brief Draws when Visible and commits actual clamped user changes. */
    void Draw() override;
    /** @brief Assignable user-value callback; programmatic Set() is silent. */
    ValueChangedHandler OnValueChanged;
    /** @brief Disables text editing and both step buttons; polling continues. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<int>& bound_;
    int step_;
    int min_;
    int max_;
};

/**
 * @brief Two-way RGBA color picker with an alpha bar.
 *
 * Components conventionally use [0, 1]; construction does not validate or
 * normalize property values. The borrowed property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT ColorPickerWidget : public Widget {
public:
    /** @brief Red, green, blue, and alpha components, in that order. */
    using Color = std::array<float, 4>;
    /** @brief Receives committed RGBA; the reference is valid only during the call. */
    using ColorChangedHandler = std::function<void(const Color&)>;
    /**
     * @brief Creates an RGBA picker with an optional user-event handler.
     * @param label ImGui label and identifier.
     * @param bound Borrowed RGBA property, required to outlive this widget.
     * @param onColorChanged Optional render-thread callback after property commit.
     */
    ColorPickerWidget(std::string label, mvvm::ObservableProperty<Color>& bound,
                      ColorChangedHandler onColorChanged = {});
    /** @brief Draws when Visible and commits actual user color changes. */
    void Draw() override;
    /** @brief Assignable user-color callback; programmatic Set() is silent. */
    ColorChangedHandler OnColorChanged;
private:
    std::string label_;
    mvvm::ObservableProperty<Color>& bound_;
};

/**
 * @brief Multiline UTF-8 text input backed by a dynamically resized buffer.
 *
 * Long text is not truncated to a fixed buffer size. Text is passed to ImGui as
 * a null-terminated string, so embedded null bytes are not supported as editable
 * text. ImGui chooses the default multiline area size. The property must outlive it.
 */
class CPPTOOLKIT_UI_EXPORT TextAreaWidget : public Widget {
public:
    /** @brief Receives committed text; the reference is valid only during the call. */
    using TextChangedHandler = std::function<void(const std::string&)>;
    /**
     * @brief Creates multiline input with an optional user-event handler.
     * @param label ImGui label and identifier.
     * @param bound Borrowed UTF-8 text property, required to outlive this widget.
     * @param onTextChanged Optional render-thread callback after property commit.
     */
    TextAreaWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                   TextChangedHandler onTextChanged = {});
    /** @brief Draws when Visible and commits actual user text changes. */
    void Draw() override;
    /** @brief Assignable user-text callback; programmatic Set() is silent. */
    TextChangedHandler OnTextChanged;
    /** @brief Prevents user text edits while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& bound_;
    std::vector<char> buffer_;
};

/**
 * @brief Masked single-line UTF-8 input with a dynamically resized buffer.
 *
 * Masking affects presentation only: property and edit buffers store plaintext,
 * and callbacks receive plaintext. This widget provides neither encryption nor
 * secure storage or memory erasure. Embedded null bytes are not supported as
 * editable text. The borrowed property must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT PasswordWidget : public Widget {
public:
    /** @brief Receives committed plaintext; the reference lasts only for the call. */
    using TextChangedHandler = std::function<void(const std::string&)>;
    /**
     * @brief Creates masked input with an optional user-event handler.
     * @param label ImGui label and identifier.
     * @param bound Borrowed plaintext UTF-8 property, required to outlive this widget.
     * @param onTextChanged Optional render-thread callback after property commit.
     */
    PasswordWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                   TextChangedHandler onTextChanged = {});
    /** @brief Draws masked text when Visible and commits actual user edits. */
    void Draw() override;
    /** @brief Assignable plaintext user-text callback; programmatic Set() is silent. */
    TextChangedHandler OnTextChanged;
    /** @brief Prevents user text edits while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& bound_;
    std::vector<char> buffer_;
};

/**
 * @brief Single-line UTF-8 input with a fixed "Search..." empty-value hint.
 *
 * This widget edits a query; it does not perform searching or filtering.
 * Its dynamically resized buffer supports long input without fixed truncation.
 * Embedded null bytes are not supported as editable text. The borrowed property
 * must outlive the widget.
 */
class CPPTOOLKIT_UI_EXPORT SearchBoxWidget : public Widget {
public:
    /** @brief Receives committed query text; the reference lasts only for the call. */
    using TextChangedHandler = std::function<void(const std::string&)>;
    /**
     * @brief Creates search-query input with an optional user-event handler.
     * @param label ImGui label and identifier.
     * @param bound Borrowed UTF-8 query property, required to outlive this widget.
     * @param onTextChanged Optional render-thread callback after property commit.
     */
    SearchBoxWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                    TextChangedHandler onTextChanged = {});
    /** @brief Draws when Visible and commits actual user query changes. */
    void Draw() override;
    /** @brief Assignable user-query callback; programmatic Set() is silent. */
    TextChangedHandler OnTextChanged;
    /** @brief Prevents user text edits while continuing to poll the property. */
    bool readOnly = false;
private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& bound_;
    std::vector<char> buffer_;
};

} // namespace cpptoolkit::ui
