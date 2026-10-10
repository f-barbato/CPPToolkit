#include "cpptoolkit/ui/widgets/OverlayWidgets.h"

#include <utility>

#include <imgui.h>

namespace cpptoolkit::ui {

void TooltipWidget::Draw() {
    if (Visible && ImGui::BeginItemTooltip()) {
        DrawChildren();
        ImGui::EndTooltip();
    }
}

PopupWidget::PopupWidget(std::string label, mvvm::ObservableProperty<bool>& open, PopupKind kind)
    : label_(std::move(label)), open_(open), kind_(kind) {}

void PopupWidget::Draw() {
    if (!Visible) return;
    const bool requested = open_.Get();
    const bool programmatic = requested != lastRequested_;
    ImGui::PushID(this);
    if (requested && programmatic) ImGui::OpenPopup(label_.c_str());

    bool titleOpen = true;
    bool shown = false;
    if (kind_ == PopupKind::Modal)
        shown = ImGui::BeginPopupModal(label_.c_str(), &titleOpen, ImGuiWindowFlags_AlwaysAutoResize);
    else if (kind_ == PopupKind::ContextMenu)
        shown = ImGui::BeginPopupContextItem(label_.c_str());
    else
        shown = ImGui::BeginPopup(label_.c_str());

    if (shown) {
        if (!requested && programmatic) {
            ImGui::CloseCurrentPopup();
        } else {
            if (!wasOpen_ && !requested) {
                open_.Set(true);
                if (OnOpened) OnOpened();
            }
            DrawChildren();
            // Children may close the popup through a Command that updates the property.
            if (!open_.Get()) ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    const bool nowOpen = titleOpen && ImGui::IsPopupOpen(label_.c_str());
    if ((wasOpen_ || shown) && !nowOpen) {
        const bool externallyClosed = programmatic && !requested;
        if (open_.Get()) open_.Set(false);
        if (!externallyClosed && OnClosed) OnClosed();
    }
    wasOpen_ = nowOpen;
    lastRequested_ = open_.Get();
    ImGui::PopID();
}

DialogWidget::DialogWidget(std::string label, mvvm::ObservableProperty<bool>& open)
    : PopupWidget(std::move(label), open, PopupKind::Modal) {}

ContextMenuWidget::ContextMenuWidget(std::string label, mvvm::ObservableProperty<bool>& open)
    : PopupWidget(std::move(label), open, PopupKind::ContextMenu) {}

} // namespace cpptoolkit::ui
