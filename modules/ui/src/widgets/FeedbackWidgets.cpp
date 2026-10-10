#include "cpptoolkit/ui/widgets/FeedbackWidgets.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace cpptoolkit::ui {

ProgressBarWidget::ProgressBarWidget(std::string label, mvvm::ObservableProperty<float>& progress,
                                     ImVec2 size)
    : label_(std::move(label)), progress_(progress), size_(size) {}

void ProgressBarWidget::Draw() {
    const float progress = progress_.Get();
    if (!std::isfinite(progress)) {
        ImGui::TextUnformatted("Invalid progress value");
        return;
    }
    ImGui::ProgressBar(std::clamp(progress, 0.0f, 1.0f), size_, label_.c_str());
}

SpinnerWidget::SpinnerWidget(std::string label, mvvm::ObservableProperty<bool>& busy)
    : label_(std::move(label)), busy_(busy) {}

void SpinnerWidget::Draw() {
    if (!busy_.Get()) return;
    const float radius = ImGui::GetTextLineHeight() * 0.4f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 center(origin.x + radius, origin.y + radius);
    auto* draw = ImGui::GetWindowDrawList();
    draw->PathClear();
    const float start = static_cast<float>(ImGui::GetTime() * 5.0);
    draw->PathArcTo(center, radius, start, start + 4.7f, 24);
    draw->PathStroke(ImGui::GetColorU32(ImGuiCol_ButtonHovered), 0, 2.0f);
    ImGui::Dummy(ImVec2(radius * 2, radius * 2));
    ImGui::SameLine();
    ImGui::TextUnformatted(label_.c_str());
}

BadgeWidget::BadgeWidget(mvvm::ObservableProperty<std::string>& text, ImVec4 color)
    : text_(text), color_(color) {}

void BadgeWidget::Draw() {
    const auto text = text_.Get();
    ImGui::TextColored(color_, "%s", text.c_str());
}

NotificationWidget::NotificationWidget(std::string label,
                                       mvvm::ObservableProperty<std::string>& message,
                                       mvvm::ObservableProperty<bool>& open)
    : label_(std::move(label)), message_(message), open_(open) {}

void NotificationWidget::Draw() {
    if (!open_.Get()) return;
    ImGui::PushID(this);
    ImGui::TextUnformatted(label_.c_str());
    ImGui::SameLine();
    const auto message = message_.Get();
    ImGui::TextUnformatted(message.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton("Dismiss")) {
        open_.Set(false);
        if (OnDismissed) OnDismissed();
    }
    ImGui::PopID();
}

ImageWidget::ImageWidget(mvvm::ObservableProperty<ImTextureID>& texture, ImVec2 size)
    : texture_(texture), size_(size) {
    if (!std::isfinite(size.x) || !std::isfinite(size.y) || size.x <= 0 || size.y <= 0)
        throw std::invalid_argument("ImageWidget requires a positive finite size");
}

void ImageWidget::Draw() {
    ImGui::Image(texture_.Get(), size_);
}

} // namespace cpptoolkit::ui
