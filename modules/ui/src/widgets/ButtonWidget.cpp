#include "cpptoolkit/ui/widgets/ButtonWidget.h"

namespace cpptoolkit::ui{
    
    ButtonWidget::ButtonWidget(std::string label, std::function<void()> onClick)
        : label_(std::move(label)), onClick_(std::move(onClick)) {}

    ButtonWidget::ButtonWidget(std::string label, mvvm::Command& command)
        : label_(std::move(label)), command_(&command) {}

    void ButtonWidget::Draw(){
        bool enabled = !command_ || command_->CanExecute();
        ImGui::BeginDisabled(!enabled);
        if (ImGui::Button(label_.c_str())) {
            if (command_) command_->Execute();
            else if (onClick_) onClick_();
        }
        ImGui::EndDisabled();
    }
}
