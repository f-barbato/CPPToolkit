#pragma once

#include <functional>

namespace cpptoolkit::mvvm {

// Analogous to WPF/C#'s ICommand: wraps an action plus an optional guard.
class Command {
public:
    explicit Command(std::function<void()> action, std::function<bool()> canExecute = nullptr)
        : action_(std::move(action)), canExecute_(std::move(canExecute)) {}

    [[nodiscard]] bool CanExecute() const { return !canExecute_ || canExecute_(); }

    void Execute() {
        if (CanExecute() && action_) action_();
    }

private:
    std::function<void()> action_;
    std::function<bool()> canExecute_;
};

} // namespace cpptoolkit::mvvm
