#pragma once
#include <cpptoolkit/mvvm/Export.h>

#include <functional>
#include <mutex>
#include <vector>

/**
 * @file Dispatcher.h
 * @brief Queues actions for execution by a caller-selected thread.
 */

namespace cpptoolkit::mvvm {

/**
 * @brief Queues actions for execution by the thread calling ProcessPending().
 *
 * Main() returns the process-wide dispatcher. Post() is safe to call while
 * another thread processes the queue. ProcessPending() swaps the queue into a
 * local batch before invoking actions, so actions posted during processing
 * wait for the next call. Actions in a batch execute in post order on the
 * processing thread.
 */
class CPPTOOLKIT_MVVM_EXPORT Dispatcher {
public:
    /**
     * @brief Returns the process-wide dispatcher instance.
     * @return Reference to the shared dispatcher.
     */
    static Dispatcher& Main();

    /**
     * @brief Adds an action to the pending queue.
     * @param action Callable to be invoked by a later ProcessPending() call.
     */
    void Post(std::function<void()> action);

    /**
     * @brief Executes the actions pending at the start of this call.
     * @throws Any exception thrown by an action; later actions in the local
     * batch are not invoked after an exception.
     */
    void ProcessPending();

private:
    std::mutex mutex_;
    std::vector<std::function<void()>> queue_;
};

} // namespace cpptoolkit::mvvm
