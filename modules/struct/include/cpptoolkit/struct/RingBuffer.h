#pragma once

#include <array>
#include <cstddef>
#include <optional>

namespace cpptoolkit::structs {

// Fixed-capacity circular buffer. Overwrites the oldest element once full.
// Useful for buffering telemetry samples at a bounded memory cost.
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0, "RingBuffer capacity must be greater than zero");

public:
    void Push(const T& value) {
        data_[head_] = value;
        head_ = (head_ + 1) % Capacity;
        if (size_ < Capacity) {
            ++size_;
        } else {
            tail_ = (tail_ + 1) % Capacity;
        }
    }

    [[nodiscard]] std::optional<T> Pop() {
        if (size_ == 0) return std::nullopt;
        T value = data_[tail_];
        tail_ = (tail_ + 1) % Capacity;
        --size_;
        return value;
    }

    [[nodiscard]] const T& At(std::size_t index) const { return data_[(tail_ + index) % Capacity]; }

    [[nodiscard]] std::size_t Size() const { return size_; }
    [[nodiscard]] constexpr std::size_t MaxSize() const { return Capacity; }
    [[nodiscard]] bool Empty() const { return size_ == 0; }
    [[nodiscard]] bool Full() const { return size_ == Capacity; }

    void Clear() {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

private:
    std::array<T, Capacity> data_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};

} // namespace cpptoolkit::structs
