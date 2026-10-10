#include "cpptoolkit/ui/widgets/ButtonWidget.h"

namespace cpptoolkit::ui{
    
    ButtonWidget::ButtonWidget(std::string label, mvvm::Command& command, std::function<void()> onClick)
        : OnClick(std::move(onClick)), label_(std::move(label)), command_(&command) {}
    ButtonWidget::ButtonWidget(std::string label, std::function<void()> onClick)
        : OnClick(std::move(onClick)), label_(std::move(label)) {}

    void ButtonWidget::Draw(){
        bool enabled = !command_ || command_->CanExecute();
        ImGui::BeginDisabled(!enabled);
        if (ImGui::Button(label_.c_str())) {
            if (command_) command_->Execute();
            if (OnClick) OnClick();
        }
        ImGui::EndDisabled();
    }
}
