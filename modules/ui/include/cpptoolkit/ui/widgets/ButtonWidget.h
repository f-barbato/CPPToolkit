#pragma once

#include <functional>
#include <string>
#include <utility>

#include <imgui.h>

#include <cpptoolkit/mvvm/Command.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Button bound to a Command (disabled whenever CanExecute() is false), or to
// a plain callback when no Command is needed. Binding is optional: pass
// neither to get an inert, always-enabled button.
class ButtonWidget : public Widget {
public:
    
    explicit ButtonWidget(std::string label, std::function<void()> onClick = nullptr);
    ButtonWidget(std::string label, mvvm::Command& command);
    void Draw() override;

private:
    std::string label_;
    mvvm::Command* command_ = nullptr;
    std::function<void()> onClick_;
};

} // namespace cpptoolkit::ui
