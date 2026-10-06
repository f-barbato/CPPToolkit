#pragma once

#include <vector>
#include <memory>
#include <algorithm>

namespace cpptoolkit::ui {

// Base class for the retained widget tree. Draw() is called every frame by
// the parent Panel (or the application main loop for the root widget) and is
// expected to issue the corresponding ImGui immediate-mode calls.
class Widget {
public:
    virtual ~Widget() = default;

    virtual void Build() {
        BuildChildren();
    }

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

    virtual void Draw() {

        if (!Visible) return;

        DrawChildren();
    }

    virtual void Destroy() {
        DestroyChildren();
    }

protected:
    void BuildChildren() {
        for (auto& child : children_) {
            child->Build();
        }
    }

    void DrawChildren() {
        for (auto& child : children_) {
            if (child->Visible) child->Draw();
        }
    }

    void DestroyChildren() {
        for (auto& child : children_) {
            child->Destroy();
        }
        children_.clear();
    }

public:
    bool Visible = true;

private:
    std::vector<std::unique_ptr<Widget>> children_;

};

} // namespace cpptoolkit::ui
