#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <unordered_map>

/**
 * @file Observable.h
 * @brief Multi-subscriber observable values and RAII subscriptions.
 */

namespace cpptoolkit::mvvm {

/**
 * @brief Notifies subscribed handlers with values supplied by the caller.
 *
 * Notify() makes a snapshot of the current handlers while holding its mutex,
 * then invokes that snapshot synchronously on the calling thread, outside the
 * mutex and in unspecified order. Subscription changes during a notification
 * affect later notifications, not the current snapshot. Concurrent Notify()
 * calls may invoke handlers concurrently on their respective calling threads.
 * Handlers may therefore reenter this observable without its mutex being held.
 * The class protects handler-map operations, but does not make handler
 * execution parallel-safe.
 *
 * A Subscription holds a non-owning pointer to this object. The Observable
 * must outlive every active Subscription. Use NotificationWorker when
 * notifications need to run on its dedicated worker thread.
 *
 * @tparam T Type of values delivered to handlers.
 */
template <typename T>
class Observable {
public:
    /** Callback invoked with a notified value. */
    using Handler = std::function<void(const T&)>;

    /** @brief RAII handle that removes one handler when reset or destroyed. */
    class Subscription {
    public:
        /** @brief Creates an inactive subscription. */
        Subscription() = default;
        /**
         * @brief Creates an active subscription handle.
         * @param owner Observable that owns the handler; it must outlive this
         * handle while the handle remains active.
         * @param id Handler identifier to remove on reset.
         */
        Subscription(Observable* owner, std::size_t id) : owner_(owner), id_(id) {}

        /**
         * @brief Transfers the active handle from another subscription.
         * @param other Subscription from which to take ownership.
         */
        Subscription(Subscription&& other) noexcept : owner_(other.owner_), id_(other.id_) {
            other.owner_ = nullptr;
        }
        /**
         * @brief Resets this handle, then takes ownership of another handle.
         * @param other Subscription from which to take ownership.
         * @return Reference to this subscription.
         */
        Subscription& operator=(Subscription&& other) noexcept {
            if (this != &other) {
                Reset();
                owner_ = other.owner_;
                id_ = other.id_;
                other.owner_ = nullptr;
            }
            return *this;
        }
        /** @brief Copying is disabled so one handler has one RAII handle. */
        Subscription(const Subscription&) = delete;
        /** @brief Copy assignment is disabled. */
        Subscription& operator=(const Subscription&) = delete;

        /** @brief Removes the handler if this handle is active. */
        ~Subscription() { Reset(); }

        /** @brief Unsubscribes this handle; calling it repeatedly is harmless. */
        void Reset() {
            if (owner_) {
                owner_->Unsubscribe(id_);
                owner_ = nullptr;
            }
        }

    private:
        Observable* owner_ = nullptr;
        std::size_t id_ = 0;
    };

    /**
     * @brief Registers a handler.
     * @param handler Callback to invoke for each notification.
     * @return A move-only handle whose reset or destruction unregisters the
     * handler.
     */
    [[nodiscard]] Subscription Subscribe(Handler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::size_t id = nextId_++;
        handlers_[id] = std::move(handler);
        return Subscription(this, id);
    }

    /**
     * @brief Delivers a value to a snapshot of currently registered handlers.
     * @param value Value passed by const reference to each handler.
     * @throws Any exception thrown by a handler; subsequent handlers in the
     * snapshot are not invoked after an exception.
     */
    void Notify(const T& value) const {
        std::unordered_map<std::size_t, Handler> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = handlers_;
        }
        for (const auto& [id, handler] : snapshot) {
            if (handler) handler(value);
        }
    }

private:
    void Unsubscribe(std::size_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.erase(id);
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::size_t, Handler> handlers_;
    std::size_t nextId_ = 0;
};

} // namespace cpptoolkit::mvvm
