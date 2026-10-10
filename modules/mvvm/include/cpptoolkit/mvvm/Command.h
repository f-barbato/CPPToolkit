#pragma once

#include <functional>

/**
 * @file Command.h
 * @brief Action wrapper with an optional execution predicate.
 */

namespace cpptoolkit::mvvm {

/** @brief Wraps an action and an optional predicate that enables execution. */
class Command {
public:
    /**
     * @brief Constructs a command.
     * @param action Action invoked by Execute() when allowed and non-empty.
     * @param canExecute Optional predicate; an empty predicate means the
     * command is enabled.
     */
    explicit Command(std::function<void()> action, std::function<bool()> canExecute = nullptr)
        : action_(std::move(action)), canExecute_(std::move(canExecute)) {}

    /**
     * @brief Evaluates whether the command may execute.
     * @return The predicate result, or true when no predicate was provided.
     * @throws Any exception thrown by the predicate.
     */
    [[nodiscard]] bool CanExecute() const { return !canExecute_ || canExecute_(); }

    /**
     * @brief Evaluates the guard and invokes the action when allowed.
     * @throws Any exception thrown by the predicate or invoked action.
     *
     * An empty action is ignored, even when the command is enabled.
     */
    void Execute() {
        if (CanExecute() && action_) action_();
    }

private:
    std::function<void()> action_;
    std::function<bool()> canExecute_;
};

} // namespace cpptoolkit::mvvm
