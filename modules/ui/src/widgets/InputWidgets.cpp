#include <cpptoolkit/ui/widgets/InputWidgets.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include <imgui.h>

#include "../../detail/TextInput.h"

namespace cpptoolkit::ui {
namespace {

template <typename T, typename Handler>
void Commit(mvvm::ObservableProperty<T>& bound, const T& previous, const T& value,
            const Handler& handler) {
    if (value != previous) {
        bound.Set(value);
        if (handler) handler(value);
    }
}

bool ValidSelection(int selected, const std::vector<std::string>& choices) {
    return selected >= 0 && static_cast<std::size_t>(selected) < choices.size();
}

void ValidateSelection(int selected, const std::vector<std::string>& choices) {
    if (choices.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        (selected != -1 && !ValidSelection(selected, choices))) {
        throw std::invalid_argument("Selection must be -1 or an index into choices");
    }
}

void DrawChoices(mvvm::ObservableProperty<int>& bound, int selected,
                 const std::vector<std::string>& choices,
                 const std::function<void(int)>& handler) {
    for (std::size_t i = 0; i < choices.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        const bool active = selected == static_cast<int>(i);
        if (ImGui::Selectable(choices[i].c_str(), active)) {
            Commit(bound, selected, static_cast<int>(i), handler);
        }
        if (active) ImGui::SetItemDefaultFocus();
        ImGui::PopID();
    }
}

} // namespace

CheckBoxWidget::CheckBoxWidget(std::string label, mvvm::ObservableProperty<bool>& bound,
                               CheckedChangedHandler handler)
    : OnCheckedChanged(std::move(handler)), label_(std::move(label)), bound_(bound) {}

void CheckBoxWidget::Draw() {
    if (!Visible) return;
    const bool previous = bound_.Get();
    bool value = previous;
    if (ImGui::Checkbox(label_.c_str(), &value)) Commit(bound_, previous, value, OnCheckedChanged);
}

RadioButtonWidget::RadioButtonWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                                     int option, SelectionChangedHandler handler)
    : OnSelectionChanged(std::move(handler)), label_(std::move(label)), bound_(bound), option_(option) {
    if (option < 0 || bound.Get() < -1) {
        throw std::invalid_argument("Radio options must be nonnegative; selection must be >= -1");
    }
}

void RadioButtonWidget::Draw() {
    if (!Visible) return;
    const int previous = bound_.Get();
    if (ImGui::RadioButton(label_.c_str(), previous == option_)) {
        Commit(bound_, previous, option_, OnSelectionChanged);
    }
}

ComboBoxWidget::ComboBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                               std::vector<std::string> choices, SelectionChangedHandler handler)
    : OnSelectionChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      choices_(std::move(choices)) {
    ValidateSelection(bound_.Get(), choices_);
}

void ComboBoxWidget::Draw() {
    if (!Visible) return;
    const int selected = bound_.Get();
    const char* preview = ValidSelection(selected, choices_) ? choices_[selected].c_str() : "";
    if (ImGui::BeginCombo(label_.c_str(), preview)) {
        DrawChoices(bound_, selected, choices_, OnSelectionChanged);
        ImGui::EndCombo();
    }
}

ListBoxWidget::ListBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                             std::vector<std::string> choices, SelectionChangedHandler handler)
    : OnSelectionChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      choices_(std::move(choices)) {
    ValidateSelection(bound_.Get(), choices_);
}

void ListBoxWidget::Draw() {
    if (!Visible) return;
    if (ImGui::BeginListBox(label_.c_str())) {
        DrawChoices(bound_, bound_.Get(), choices_, OnSelectionChanged);
        ImGui::EndListBox();
    }
}

InputIntWidget::InputIntWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                               int step, int stepFast, ValueChangedHandler handler)
    : OnValueChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      step_(step), stepFast_(stepFast) {
    if (step < 0 || stepFast < 0) throw std::invalid_argument("Input steps must be nonnegative");
}

void InputIntWidget::Draw() {
    if (!Visible) return;
    const int previous = bound_.Get();
    int value = previous;
    if (ImGui::InputInt(label_.c_str(), &value, step_, stepFast_,
                        readOnly ? ImGuiInputTextFlags_ReadOnly : 0)) {
        Commit(bound_, previous, value, OnValueChanged);
    }
}

InputFloatWidget::InputFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                                   float step, float stepFast, ValueChangedHandler handler)
    : OnValueChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      step_(step), stepFast_(stepFast) {
    if (!std::isfinite(step) || !std::isfinite(stepFast) || step < 0 || stepFast < 0) {
        throw std::invalid_argument("Input steps must be finite and nonnegative");
    }
}

void InputFloatWidget::Draw() {
    if (!Visible) return;
    const float previous = bound_.Get();
    float value = previous;
    if (ImGui::InputFloat(label_.c_str(), &value, step_, stepFast_, "%.3f",
                          readOnly ? ImGuiInputTextFlags_ReadOnly : 0)) {
        Commit(bound_, previous, value, OnValueChanged);
    }
}

DragFloatWidget::DragFloatWidget(std::string label, mvvm::ObservableProperty<float>& bound,
                                 float speed, float min, float max, ValueChangedHandler handler)
    : OnValueChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      speed_(speed), min_(min), max_(max) {
    if (!std::isfinite(speed) || speed <= 0 || !std::isfinite(min) ||
        !std::isfinite(max) || min > max) {
        throw std::invalid_argument("Drag speed must be positive and bounds finite and ordered");
    }
}

void DragFloatWidget::Draw() {
    if (!Visible) return;
    const float previous = bound_.Get();
    float value = previous;
    if (ImGui::DragFloat(label_.c_str(), &value, speed_, min_, max_, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp)) {
        Commit(bound_, previous, std::clamp(value, min_, max_), OnValueChanged);
    }
}

SpinBoxWidget::SpinBoxWidget(std::string label, mvvm::ObservableProperty<int>& bound,
                             int step, int min, int max, ValueChangedHandler handler)
    : OnValueChanged(std::move(handler)), label_(std::move(label)), bound_(bound),
      step_(step), min_(min), max_(max) {
    if (step <= 0 || min > max) {
        throw std::invalid_argument("Spin step must be positive and bounds ordered");
    }
}

void SpinBoxWidget::Draw() {
    if (!Visible) return;
    const int previous = bound_.Get();
    int value = previous;
    // Step arithmetic uses 64 bits so a button cannot overflow an int at either limit.
    ImGui::PushID(this);
    ImGui::BeginDisabled(readOnly);
    bool changed = ImGui::InputInt(label_.c_str(), &value, 0, 0,
                                   readOnly ? ImGuiInputTextFlags_ReadOnly : 0);
    ImGui::SameLine();
    if (ImGui::SmallButton("-")) {
        value = static_cast<int>(std::clamp<std::int64_t>(
            static_cast<std::int64_t>(value) - step_, min_, max_));
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("+")) {
        value = static_cast<int>(std::clamp<std::int64_t>(
            static_cast<std::int64_t>(value) + step_, min_, max_));
        changed = true;
    }
    ImGui::EndDisabled();
    ImGui::PopID();
    if (changed) Commit(bound_, previous, std::clamp(value, min_, max_), OnValueChanged);
}

ColorPickerWidget::ColorPickerWidget(std::string label, mvvm::ObservableProperty<Color>& bound,
                                     ColorChangedHandler handler)
    : OnColorChanged(std::move(handler)), label_(std::move(label)), bound_(bound) {}

void ColorPickerWidget::Draw() {
    if (!Visible) return;
    const auto previous = bound_.Get();
    auto value = previous;
    if (ImGui::ColorPicker4(label_.c_str(), value.data(), ImGuiColorEditFlags_AlphaBar)) {
        Commit(bound_, previous, value, OnColorChanged);
    }
}

TextAreaWidget::TextAreaWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                               TextChangedHandler handler)
    : OnTextChanged(std::move(handler)), label_(std::move(label)), bound_(bound) {}

void TextAreaWidget::Draw() {
    if (Visible) detail::DrawText(label_, bound_, buffer_, readOnly, detail::TextMode::Multiline, OnTextChanged);
}

PasswordWidget::PasswordWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                               TextChangedHandler handler)
    : OnTextChanged(std::move(handler)), label_(std::move(label)), bound_(bound) {}

void PasswordWidget::Draw() {
    if (Visible) detail::DrawText(label_, bound_, buffer_, readOnly, detail::TextMode::Password, OnTextChanged);
}

SearchBoxWidget::SearchBoxWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                                 TextChangedHandler handler)
    : OnTextChanged(std::move(handler)), label_(std::move(label)), bound_(bound) {}

void SearchBoxWidget::Draw() {
    if (Visible) detail::DrawText(label_, bound_, buffer_, readOnly, detail::TextMode::Search, OnTextChanged);
}

} // namespace cpptoolkit::ui
