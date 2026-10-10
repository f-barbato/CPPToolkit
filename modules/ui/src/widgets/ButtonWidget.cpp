#include "cpptoolkit/ui/widgets/ButtonWidget.h"

namespace cpptoolkit::ui{
    
    ButtonWidget::ButtonWidget(std::string label, mvvm::Command& command, std::function<void()> onClick)
        : OnClick(std::move(onClick)), label_(std::move(label)), command_(command) {}

    void ButtonWidget::Draw(){
        ImGui::BeginDisabled(!command_.CanExecute());
        if (ImGui::Button(label_.c_str())) {
            command_.Execute();
            if (OnClick) OnClick();
        }
        ImGui::EndDisabled();
    }
}
