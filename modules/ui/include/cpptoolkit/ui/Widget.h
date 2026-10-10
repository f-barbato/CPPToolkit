#pragma once

/** @file
 *  @brief Base ownership and lifecycle API for retained widget trees.
 */

#include <vector>
#include <memory>
#include <algorithm>
#include <utility>

namespace cpptoolkit::ui {

/**
 * @brief Base class owning a persistent tree of immediate-mode widgets.
 *
 * Build(), Draw(), Destroy(), child mutations and event assignments must run
 * on the render thread. Defer tree mutations until the current traversal has
 * finished. A child's references to properties/commands are borrowed.
 */
class Widget {
public:
    /** @brief Destroy this widget and its owned children. */
    virtual ~Widget() = default;

    /** @brief Build all children, including hidden ones. */
    virtual void Build() {
        BuildChildren();
    }

    /**
     * @brief Construct and own a child widget.
     * @tparam T Widget-derived child type.
     * @tparam Args Constructor argument types.
     * @param args Arguments forwarded to the child's constructor.
     * @return Reference valid until that child is removed or its owner destroyed.
     */
    template <typename T, typename... Args>
    T& Add(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        children_.push_back(std::move(widget));
        return ref;
    }

    /** @brief Destroy a direct child if it belongs to this widget.
     *  @param widget Direct child to remove; non-children leave the tree unchanged.
     */
    void Remove(Widget& widget) {
        children_.erase(
            std::remove_if(children_.begin(), children_.end(),
                            [&widget](const std::unique_ptr<Widget>& ptr) { return ptr.get() == &widget; }),
            children_.end());
    }

    /** @brief Draw visible children using the current ImGui context.
     *  @note Derived leaf Draw() implementations may rely on the parent to check Visible.
     */
    virtual void Draw() {

        if (!Visible) return;

        DrawChildren();
    }

    /** @brief Call child Destroy() hooks and release all children. */
    virtual void Destroy() {
        DestroyChildren();
    }

protected:
    /** @brief Visit visible direct children in insertion order.
     *  @tparam Action Callable accepting a Widget reference.
     *  @param action Visitor; must not mutate this child collection while iterating.
     */
    template <typename Action>
    void ForEachVisibleChild(Action&& action) {
        for (auto& child : children_) {
            if (child->Visible) action(*child);
        }
    }

    /** @brief Invoke Build() on every child in insertion order. */
    void BuildChildren() {
        for (auto& child : children_) {
            child->Build();
        }
    }

    /** @brief Draw direct children whose Visible flag is true. */
    void DrawChildren() {
        for (auto& child : children_) {
            if (child->Visible) child->Draw();
        }
    }

    /** @brief Invoke child teardown hooks before destroying the children. */
    void DestroyChildren() {
        for (auto& child : children_) {
            child->Destroy();
        }
        children_.clear();
    }

public:
    /** @brief Whether parent traversal submits this widget for drawing. */
    bool Visible = true;

private:
    std::vector<std::unique_ptr<Widget>> children_;

};

} // namespace cpptoolkit::ui
