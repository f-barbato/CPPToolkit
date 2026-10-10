#pragma once

#include <array>
#include <cstddef>
#include <optional>

/**
 * @file RingBuffer.h
 * @brief Fixed-capacity circular buffer.
 */

namespace cpptoolkit::structs {

/**
 * @brief Stores up to Capacity values in a fixed-size circular buffer.
 *
 * When full, Push() replaces the oldest value. The buffer is not thread-safe.
 * At() does not check the logical size: its index is reduced modulo Capacity
 * and can refer to a slot that does not currently contain a queued value.
 *
 * @tparam T Stored value type.
 * @tparam Capacity Number of storage slots; must be greater than zero.
 */
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0, "RingBuffer capacity must be greater than zero");

public:
    /**
     * @brief Appends a value, overwriting the oldest value if the buffer is full.
     * @param value Value to copy into the buffer.
     */
    void Push(const T& value) {
        data_[head_] = value;
        head_ = (head_ + 1) % Capacity;
        if (size_ < Capacity) {
            ++size_;
        } else {
            tail_ = (tail_ + 1) % Capacity;
        }
    }

    /**
     * @brief Removes and returns the oldest queued value.
     * @return The removed value, or std::nullopt when the buffer is empty.
     */
    [[nodiscard]] std::optional<T> Pop() {
        if (size_ == 0) return std::nullopt;
        T value = data_[tail_];
        tail_ = (tail_ + 1) % Capacity;
        --size_;
        return value;
    }

    /**
     * @brief Returns the storage slot at a wrapped index.
     * @param index Index relative to the oldest queued value.
     * @return A const reference to the slot at (oldest slot + index) modulo
     * Capacity. The index is not checked against Size(); an index outside the
     * queued range can therefore refer to an unused or stale slot.
     */
    [[nodiscard]] const T& At(std::size_t index) const { return data_[(tail_ + index) % Capacity]; }

    /**
     * @brief Returns the number of queued values.
     * @return Number of values currently queued, from zero through Capacity.
     */
    [[nodiscard]] std::size_t Size() const { return size_; }
    /**
     * @brief Returns the compile-time storage capacity.
     * @return Capacity template argument.
     */
    [[nodiscard]] constexpr std::size_t MaxSize() const { return Capacity; }
    /**
     * @brief Reports whether no values are queued.
     * @return True if Size() is zero.
     */
    [[nodiscard]] bool Empty() const { return size_ == 0; }
    /**
     * @brief Reports whether all storage slots currently hold queued values.
     * @return True if Size() equals Capacity.
     */
    [[nodiscard]] bool Full() const { return size_ == Capacity; }

    /** @brief Removes all queued values without modifying the storage slots. */
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
