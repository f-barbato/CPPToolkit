#pragma once

#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/Command.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Button bound to a Command: disabled whenever Command::CanExecute() is false.
class ButtonWidget : public Widget {
public:
    ButtonWidget(std::string label, mvvm::Command& command) : label_(std::move(label)), command_(command) {}

    void Draw() override {
        ImGui::BeginDisabled(!command_.CanExecute());
        if (ImGui::Button(label_.c_str())) {
            command_.Execute();
        }
        ImGui::EndDisabled();
    }

private:
    std::string label_;
    mvvm::Command& command_;
};

} // namespace cpptoolkit::ui
