#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

#include "cpptoolkit/mvvm/ObservableObject.h"

namespace cpptoolkit::ui {

// Base class for the retained widget tree. Draw() is called every frame by
// the parent Panel (or the application main loop for the root widget) and is
// expected to issue the corresponding ImGui immediate-mode calls.
//
// Data binding context: a ViewModel is attached once (typically at the tree
// root, e.g. by View<T>) via SetViewModel(), and automatically cascades to
// every widget subsequently Add()-ed beneath it (see Add() below) unless a
// child already has one of its own. Binding is therefore fully optional and
// "ambient": a descendant that needs to bind to a specific property/command
// just calls GetViewModel<ConcreteViewModel>() to fetch it (nullptr if none
// is set, or the type doesn't match) instead of having the ViewModel
// threaded manually through every intermediate constructor.
class Widget {
public:
    virtual ~Widget() = default;

    virtual void PreBuild() {
    }

    // Builds this widget's subtree: runs the user-overridable OnBuild() hook
    // (where widgets normally Add<>() their children), then recurses into
    // whatever children were just added. Not virtual on purpose: subclasses
    // customize construction via OnBuild(), not Build() itself, so the
    // recursion step can never accidentally be forgotten.
    void Build() {
        OnBuild();
        BuildChildren();
    }

    template <typename T, typename... Args>
    T& Add(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        if (!widget->HasViewModel()) widget->SetViewModel(viewModel_);
        widget->PreBuild();
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

    // Attaches (or overrides) the ambient data-binding context for this
    // widget. Safe to call with nullptr to explicitly opt out of binding.
    void SetViewModel(mvvm::ObservableObject* viewModel) { viewModel_ = viewModel; }

    bool HasViewModel() const { return viewModel_ != nullptr; }

    // Fetches the ambient ViewModel cast to the concrete type a widget
    // actually needs for binding. Returns nullptr if none is set or the
    // type doesn't match, so callers can treat binding as optional.
    template <typename T>
    T* GetViewModel() const { return dynamic_cast<T*>(viewModel_); }

protected:

    // Hook for subclasses to build their child tree (Add<>() calls); called
    // once by Build(), before children are recursively built. Defaults to a
    // no-op for leaf widgets that don't have children of their own.
    virtual void OnBuild() {}

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
    mvvm::ObservableObject* viewModel_ = nullptr;

};

} // namespace cpptoolkit::ui
