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
    
    ButtonWidget(std::string label, mvvm::Command& command);
    void Draw() override;

private:
    std::string label_;
    mvvm::Command& command_;
};

} // namespace cpptoolkit::ui
