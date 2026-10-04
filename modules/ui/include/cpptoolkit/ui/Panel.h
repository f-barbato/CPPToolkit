#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Persistent container of child widgets, built once and drawn every frame.
// This is what turns ImGui's immediate-mode calls into a "retained" tree:
// the structure is created up front (Add/Remove), not rebuilt per frame.
class Panel : public Widget {
public:
    template <typename T, typename... Args>
    T& Add(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        children_.push_back(std::move(widget));
        return ref;
    }

    void Remove(Widget& widget) {
        children_.erase(
            std::remove_if(children_.begin(), children_.end(),
                            [&widget](const std::unique_ptr<Widget>& ptr) { return ptr.get() == &widget; }),
            children_.end());
    }

    void Draw() override {
        for (auto& child : children_) {
            if (child->Visible) child->Draw();
        }
    }

private:
    std::vector<std::unique_ptr<Widget>> children_;
};

} // namespace cpptoolkit::ui
