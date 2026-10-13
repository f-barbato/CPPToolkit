#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cpptoolkit/mvvm/Command.h"
#include "cpptoolkit/mvvm/ObservableProperty.h"

/**
 * @file UndoStack.h
 * @brief Undo/redo history with grouping, edit merging and bindable state.
 */

namespace cpptoolkit::mvvm {

/** @brief Reversible operation stored in an UndoStack. */
class IUndoableAction {
public:
    virtual ~IUndoableAction() = default;
    /** @brief Apply (or re-apply) the operation. */
    virtual void Do() = 0;
    /** @brief Revert the operation. */
    virtual void Undo() = 0;
    /** @brief Human-readable description shown in "Undo <description>" menus.
     *  @return Description, possibly empty.
     */
    [[nodiscard]] virtual std::string Description() const { return {}; }
    /** @brief Absorb a following action so both become a single undo step.
     *  @param next Action recorded right after this one.
     *  @return True if next was merged into this action and must be discarded.
     */
    virtual bool TryMerge(const IUndoableAction& next) {
        (void)next;
        return false;
    }
};

/** @brief Action defined by two callables. */
class DelegateAction final : public IUndoableAction {
public:
    /**
     * @brief Create an action from callables.
     * @param description Text for menus.
     * @param doAction Applies the operation; must be non-empty.
     * @param undoAction Reverts the operation; must be non-empty.
     * @throws std::invalid_argument If a callable is empty.
     */
    DelegateAction(std::string description, std::function<void()> doAction, std::function<void()> undoAction)
        : description_(std::move(description)), do_(std::move(doAction)), undo_(std::move(undoAction)) {
        if (!do_ || !undo_) throw std::invalid_argument("DelegateAction requires both callables");
    }
    void Do() override { do_(); }
    void Undo() override { undo_(); }
    [[nodiscard]] std::string Description() const override { return description_; }

private:
    std::string description_;
    std::function<void()> do_;
    std::function<void()> undo_;
};

/**
 * @brief Changes an ObservableProperty from an old to a new value.
 *
 * The property is borrowed and must outlive every stack entry referring to it
 * (clear the stack before destroying it). Consecutive mergeable changes of the
 * same property collapse into one step, e.g. while dragging a slider.
 */
template <typename T>
class PropertyChangeAction final : public IUndoableAction {
public:
    /**
     * @brief Create a property change.
     * @param property Borrowed property.
     * @param oldValue Value restored by Undo().
     * @param newValue Value applied by Do().
     * @param description Text for menus.
     * @param mergeable Whether following changes of the same property may merge into this one.
     */
    PropertyChangeAction(ObservableProperty<T>& property, T oldValue, T newValue,
                         std::string description = {}, bool mergeable = false)
        : property_(property), old_(std::move(oldValue)), new_(std::move(newValue)),
          description_(std::move(description)), mergeable_(mergeable) {}
    void Do() override { property_.Set(new_); }
    void Undo() override { property_.Set(old_); }
    [[nodiscard]] std::string Description() const override {
        return description_.empty() ? property_.Name() : description_;
    }
    bool TryMerge(const IUndoableAction& next) override {
        const auto* other = dynamic_cast<const PropertyChangeAction*>(&next);
        if (!other || &other->property_ != &property_ || !mergeable_ || !other->mergeable_) return false;
        new_ = other->new_;
        return true;
    }

private:
    ObservableProperty<T>& property_;
    T old_;
    T new_;
    std::string description_;
    bool mergeable_;
};

/**
 * @brief Linear undo/redo history.
 *
 * Execute() applies an action and records it; Record() records an operation
 * that has already been applied (typical for widgets that commit first).
 * A new action clears the redo history. Calls made while an undo/redo/execute
 * is applying (for example from property subscribers) are not recorded.
 * Not thread-safe: use it from one thread, typically the render thread.
 * Actions may hold borrowed references; Clear() the stack before they dangle.
 */
class UndoStack {
public:
    /**
     * @brief Create an empty history.
     * @param maxDepth Maximum retained undo steps; the oldest are dropped.
     * @throws std::invalid_argument If maxDepth is zero.
     */
    explicit UndoStack(std::size_t maxDepth = 100)
        : maxDepth_(maxDepth),
          UndoCommand([this] { Undo(); }, [this] { return CanUndo.Get(); }),
          RedoCommand([this] { Redo(); }, [this] { return CanRedo.Get(); }) {
        if (maxDepth == 0) throw std::invalid_argument("UndoStack depth must be positive");
    }
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    /** @brief Apply an action and record it.
     *  @param action Action to run; recorded only if Do() returns normally.
     */
    void Execute(std::unique_ptr<IUndoableAction> action) {
        if (!action) return;
        if (applying_) {
            action->Do();
            return;
        }
        {
            ApplyScope scope(*this);
            action->Do();
        }
        Push(std::move(action));
    }
    /** @brief Apply an operation defined by callables and record it.
     *  @param description Menu text.
     *  @param doAction Applies the operation.
     *  @param undoAction Reverts the operation.
     */
    void Execute(std::string description, std::function<void()> doAction, std::function<void()> undoAction) {
        Execute(std::make_unique<DelegateAction>(std::move(description), std::move(doAction), std::move(undoAction)));
    }
    /** @brief Record an operation that is already applied; Do() is not called.
     *  @param action Action whose effect is already in place.
     */
    void Record(std::unique_ptr<IUndoableAction> action) {
        if (!action || applying_) return;
        Push(std::move(action));
    }
    /** @brief Revert the latest step; no effect when empty or while applying.
     *  @throws Any exception from the action; the history stays unchanged then.
     */
    void Undo() {
        if (applying_ || undo_.empty() || !groups_.empty()) return;
        {
            ApplyScope scope(*this);
            undo_.back()->Undo();
        }
        redo_.push_back(std::move(undo_.back()));
        undo_.pop_back();
        barrier_ = true;
        UpdateState();
    }
    /** @brief Re-apply the latest reverted step; no effect when empty or while applying.
     *  @throws Any exception from the action; the history stays unchanged then.
     */
    void Redo() {
        if (applying_ || redo_.empty() || !groups_.empty()) return;
        {
            ApplyScope scope(*this);
            redo_.back()->Do();
        }
        undo_.push_back(std::move(redo_.back()));
        redo_.pop_back();
        barrier_ = true;
        UpdateState();
    }
    /** @brief Forget all history and open groups. */
    void Clear() {
        undo_.clear();
        redo_.clear();
        groups_.clear();
        barrier_ = false;
        UpdateState();
    }
    /** @brief Stop the next recorded action from merging into the previous one.
     *  @note Call when a continuous edit ends (e.g. slider released).
     */
    void BreakMerge() { barrier_ = true; }

    /** @brief Start collecting actions into one undo step; calls may nest.
     *  @param description Menu text of the resulting step.
     */
    void BeginGroup(std::string description = {}) { groups_.push_back({std::move(description), {}}); }
    /** @brief Close the innermost group, recording it as a single step if non-empty. */
    void EndGroup() {
        if (groups_.empty()) return;
        auto frame = std::move(groups_.back());
        groups_.pop_back();
        if (frame.actions.empty()) return;
        auto composite = std::make_unique<Composite>(std::move(frame.description), std::move(frame.actions));
        if (!groups_.empty()) groups_.back().actions.push_back(std::move(composite));
        else Push(std::move(composite));
    }
    /** @brief Query whether a group is open.
     *  @return True while BeginGroup() has not been matched by EndGroup().
     */
    [[nodiscard]] bool InGroup() const { return !groups_.empty(); }

    /** @brief Number of undoable steps.
     *  @return Undo history size.
     */
    [[nodiscard]] std::size_t UndoDepth() const { return undo_.size(); }
    /** @brief Number of redoable steps.
     *  @return Redo history size.
     */
    [[nodiscard]] std::size_t RedoDepth() const { return redo_.size(); }

    /** @brief RAII group: BeginGroup() on construction, EndGroup() on destruction. */
    class Group {
    public:
        /**
         * @brief Open a group.
         * @param stack Stack to group on; must outlive this object.
         * @param description Menu text.
         */
        Group(UndoStack& stack, std::string description = {}) : stack_(stack) {
            stack_.BeginGroup(std::move(description));
        }
        Group(const Group&) = delete;
        Group& operator=(const Group&) = delete;
        ~Group() { stack_.EndGroup(); }

    private:
        UndoStack& stack_;
    };

    /** @brief True when Undo() would revert something (bindable). */
    ObservableProperty<bool> CanUndo{nullptr, "CanUndo", false};
    /** @brief True when Redo() would re-apply something (bindable). */
    ObservableProperty<bool> CanRedo{nullptr, "CanRedo", false};
    /** @brief Description of the step Undo() would revert; empty if none. */
    ObservableProperty<std::string> UndoText{nullptr, "UndoText", ""};
    /** @brief Description of the step Redo() would re-apply; empty if none. */
    ObservableProperty<std::string> RedoText{nullptr, "RedoText", ""};

private:
    struct ApplyScope {
        explicit ApplyScope(UndoStack& s) : stack(s) { stack.applying_ = true; }
        ~ApplyScope() { stack.applying_ = false; }
        UndoStack& stack;
    };

    class Composite final : public IUndoableAction {
    public:
        Composite(std::string description, std::vector<std::unique_ptr<IUndoableAction>> actions)
            : description_(std::move(description)), actions_(std::move(actions)) {}
        void Do() override {
            for (auto& action : actions_) action->Do();
        }
        void Undo() override {
            for (auto it = actions_.rbegin(); it != actions_.rend(); ++it) (*it)->Undo();
        }
        [[nodiscard]] std::string Description() const override {
            return description_.empty() && !actions_.empty() ? actions_.front()->Description() : description_;
        }

    private:
        std::string description_;
        std::vector<std::unique_ptr<IUndoableAction>> actions_;
    };

    struct GroupFrame {
        std::string description;
        std::vector<std::unique_ptr<IUndoableAction>> actions;
    };

    void Push(std::unique_ptr<IUndoableAction> action) {
        if (!groups_.empty()) {
            groups_.back().actions.push_back(std::move(action));
            return;
        }
        redo_.clear();
        if (!barrier_ && !undo_.empty() && undo_.back()->TryMerge(*action)) {
            UpdateState();
            return;
        }
        barrier_ = false;
        undo_.push_back(std::move(action));
        while (undo_.size() > maxDepth_) undo_.pop_front();
        UpdateState();
    }

    void UpdateState() {
        CanUndo.Set(!undo_.empty());
        CanRedo.Set(!redo_.empty());
        UndoText.Set(undo_.empty() ? std::string() : undo_.back()->Description());
        RedoText.Set(redo_.empty() ? std::string() : redo_.back()->Description());
    }

    std::size_t maxDepth_;
    std::deque<std::unique_ptr<IUndoableAction>> undo_;
    std::deque<std::unique_ptr<IUndoableAction>> redo_;
    std::vector<GroupFrame> groups_;
    bool applying_ = false;
    bool barrier_ = false;

public:
    /** @brief Command reverting the latest step; enabled by CanUndo. */
    Command UndoCommand;
    /** @brief Command re-applying the latest reverted step; enabled by CanRedo. */
    Command RedoCommand;
};

/**
 * @brief Set a property and record the change.
 * @tparam T Equality-comparable, copyable value type.
 * @param stack History receiving the change.
 * @param property Borrowed property; must outlive the stack entry.
 * @param value New value.
 * @param description Menu text; defaults to the property name.
 * @param mergeable Whether consecutive changes of this property merge into one step.
 * @return True if the value changed and was recorded.
 */
template <typename T>
bool SetUndoable(UndoStack& stack, ObservableProperty<T>& property, const T& value,
                 std::string description = {}, bool mergeable = false) {
    T previous = property.Get();
    if (previous == value) return false;
    stack.Execute(std::make_unique<PropertyChangeAction<T>>(property, std::move(previous), value,
                                                           std::move(description), mergeable));
    return true;
}

/**
 * @brief Record a property change that was already applied.
 * @tparam T Equality-comparable, copyable value type.
 * @param stack History receiving the change.
 * @param property Borrowed property; must outlive the stack entry.
 * @param oldValue Value before the change.
 * @param newValue Value now held by the property.
 * @param description Menu text; defaults to the property name.
 * @param mergeable Whether consecutive changes of this property merge into one step.
 */
template <typename T>
void RecordChange(UndoStack& stack, ObservableProperty<T>& property, const T& oldValue, const T& newValue,
                  std::string description = {}, bool mergeable = false) {
    if (oldValue == newValue) return;
    stack.Record(std::make_unique<PropertyChangeAction<T>>(property, oldValue, newValue,
                                                          std::move(description), mergeable));
}

} // namespace cpptoolkit::mvvm
