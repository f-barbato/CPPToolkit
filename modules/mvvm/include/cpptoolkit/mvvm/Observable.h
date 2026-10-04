#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace cpptoolkit::mvvm {

// Multi-subscriber observable with RAII subscriptions (auto-unsubscribe on
// Subscription destruction). Notify() executes handlers synchronously, on the
// calling thread, in no particular order — see NotificationWorker if you need
// to offload handler execution to a dedicated background thread.
template <typename T>
class Observable {
public:
    using Handler = std::function<void(const T&)>;

    class Subscription {
    public:
        Subscription() = default;
        Subscription(Observable* owner, std::size_t id) : owner_(owner), id_(id) {}

        Subscription(Subscription&& other) noexcept : owner_(other.owner_), id_(other.id_) {
            other.owner_ = nullptr;
        }
        Subscription& operator=(Subscription&& other) noexcept {
            if (this != &other) {
                Reset();
                owner_ = other.owner_;
                id_ = other.id_;
                other.owner_ = nullptr;
            }
            return *this;
        }
        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        ~Subscription() { Reset(); }

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

    [[nodiscard]] Subscription Subscribe(Handler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::size_t id = nextId_++;
        handlers_[id] = std::move(handler);
        return Subscription(this, id);
    }

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
