#pragma once
#include <cpptoolkit/ui/Export.h>

/** @file
 *  @brief Base ownership and lifecycle API for retained widget trees.
 */

#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
#include <exception>

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

#include "cpptoolkit/mvvm/ObservableObject.h"

namespace cpptoolkit::ui {

class Application;

/**
 * @brief Base class owning a persistent tree of immediate-mode widgets.
 *
 * Build(), Render(), Draw(), Destroy(), child mutations and event assignments must run
 * on the render thread. Defer tree mutations until the current traversal has
 * finished. A child's references to properties/commands are borrowed.
 */
class Widget {
public:
    /** @brief Destroy this widget and its owned children. */
    virtual ~Widget() = default;

    /** @brief Initialize a newly attached widget after its ambient context is assigned. */
    virtual void PreBuild() {
    }

    /** @brief Invoke OnBuild() then build children, including hidden ones.
     *  @note Kept virtual for existing containers; new views should override OnBuild().
     */
    virtual void Build() {
        OnBuild();
        BuildChildren();
    }

    /** @brief Prepare graphics and visit render-enabled children before the ImGui frame.
     *  @note Called inside raylib BeginDrawing()/EndDrawing(). No ImGui calls are
     *        allowed here. Visible, collapsed windows and inactive tabs do not gate
     *        this phase; use RenderEnabled to suspend a subtree explicitly.
     */
    virtual void Render() {
        if (!RenderEnabled) return;
        OnRender();
        RenderChildren();
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
        if (!widget->HasViewModel()) widget->SetViewModel(viewModel_);
        if (!widget->HasApplication()) widget->SetApplication(application_);
        widget->PreBuild();
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

    /** @brief Set the borrowed ambient binding context for future children.
     *  @param viewModel Model pointer, or nullptr for no model.
     */
    void SetViewModel(mvvm::ObservableObject* viewModel) { viewModel_ = viewModel; }

    /** @brief Test whether a model is attached.
     *  @return True when the borrowed model pointer is nonnull.
     */
    bool HasViewModel() const { return viewModel_ != nullptr; }

    /** @brief Retrieve a compatible ambient model.
     *  @tparam T Requested ObservableObject-derived type.
     *  @return Borrowed typed model, or nullptr if missing/incompatible.
     */
    template <typename T>
    T* GetViewModel() const { return dynamic_cast<T*>(viewModel_); }

    /** @brief Set the borrowed application context for future children.
     *  @param application Application pointer, or nullptr.
     */
    void SetApplication(Application* application) { application_ = application; }

    /** @brief Test whether an application is attached.
     *  @return True when the application pointer is nonnull.
     */
    bool HasApplication() const { return application_ != nullptr; }

    /** @brief Access the ambient application.
     *  @return Borrowed application pointer, possibly nullptr.
     */
    Application* GetApplication() const { return application_; }

protected:
    /** @brief Hook for adding children before recursive construction. */
    virtual void OnBuild() {}

    /** @brief Optional raylib rendering/texture-update hook before child rendering.
     *  @note Does not advance application simulation automatically.
     */
    virtual void OnRender() {}

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
            if (!child->HasViewModel()) child->SetViewModel(viewModel_);
            if (!child->HasApplication()) child->SetApplication(application_);
            child->Build();
        }
    }

    /** @brief Draw direct children whose Visible flag is true. */
    void DrawChildren() {
        for (auto& child : children_) {
            if (child->Visible) child->Draw();
        }

    }

    /** @brief Render direct children in insertion order, independently of Visible. */
    void RenderChildren() {
        for (auto& child : children_) {
            if (child->RenderEnabled) child->Render();
        }
    }

    /** @brief Tear down every child, release ownership, then rethrow the first failure. */
    void DestroyChildren() {
        std::exception_ptr failure;
        for (auto& child : children_) {
            try {
                child->Destroy();
            } catch (...) {
                if (!failure) failure = std::current_exception();
            }
        }
        children_.clear();
        if (failure) std::rethrow_exception(failure);
    }

public:
    /** @brief Whether parent traversal submits this widget for drawing. */
    bool Visible = true;

    /** @brief Whether the pre-ImGui render phase visits this widget and its subtree. */
    bool RenderEnabled = true;

private:
    std::vector<std::unique_ptr<Widget>> children_;
    mvvm::ObservableObject* viewModel_ = nullptr;
    Application* application_ = nullptr;

};

} // namespace cpptoolkit::ui
