#pragma once

#include <cpptoolkit/concurrency/Export.h>

#include <memory>

/**
 * @file CancellationToken.h
 * @brief Cooperative cancellation source and token.
 */

namespace cpptoolkit::concurrency {

class CancellationSource;

/**
 * @brief Read-only handle for checking a cooperative cancellation request.
 *
 * A default-constructed token cannot be cancelled. Copies share cancellation
 * state with their source and remain valid after the source is destroyed.
 */
class CPPTOOLKIT_CONCURRENCY_EXPORT CancellationToken {
public:
    /** @brief Creates a token that is not associated with a cancellation source. */
    CancellationToken();

    /**
     * @brief Checks whether cancellation has been requested.
     * @return True if the associated source has requested cancellation.
     */
    [[nodiscard]] bool IsCancellationRequested() const noexcept;

private:
    struct State;

    explicit CancellationToken(std::shared_ptr<State> state);

    std::shared_ptr<State> state_;

    friend class CancellationSource;
};

/**
 * @brief Owns cancellation state and lets its tokens observe a cancellation request.
 *
 * Cancellation is cooperative: work must periodically check a token and exit
 * safely. Calling Cancel() is thread-safe and idempotent.
 */
class CPPTOOLKIT_CONCURRENCY_EXPORT CancellationSource {
public:
    /** @brief Creates a source with cancellation not yet requested. */
    CancellationSource();

    /** @brief Sources cannot be copied; their tokens can be copied freely. */
    CancellationSource(const CancellationSource&) = delete;
    /** @brief Sources cannot be copied. */
    CancellationSource& operator=(const CancellationSource&) = delete;

    /**
     * @brief Returns a token observing this source.
     * @return A copyable token sharing the source's cancellation state.
     */
    [[nodiscard]] CancellationToken Token() const;

    /** @brief Requests cancellation for all tokens issued by this source. */
    void Cancel() noexcept;

    /**
     * @brief Checks whether cancellation has been requested.
     * @return True after Cancel() has been called.
     */
    [[nodiscard]] bool IsCancellationRequested() const noexcept;

private:
    std::shared_ptr<CancellationToken::State> state_;
};

} // namespace cpptoolkit::concurrency
