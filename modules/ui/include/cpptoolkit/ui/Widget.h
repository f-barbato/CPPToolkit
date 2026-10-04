#pragma once

namespace cpptoolkit::ui {

// Base class for the retained widget tree. Draw() is called every frame by
// the parent Panel (or the application main loop for the root widget) and is
// expected to issue the corresponding ImGui immediate-mode calls.
class Widget {
public:
    virtual ~Widget() = default;
    virtual void Draw() = 0;

    bool Visible = true;
};

} // namespace cpptoolkit::ui
