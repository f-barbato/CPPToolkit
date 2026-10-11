#include "cpptoolkit/concurrency/CancellationToken.h"

#include <atomic>
#include <utility>

namespace cpptoolkit::concurrency {

struct CancellationToken::State {
    std::atomic<bool> cancelled{false};
};

CancellationToken::CancellationToken() = default;

CancellationToken::CancellationToken(std::shared_ptr<State> state)
    : state_(std::move(state)) {}

bool CancellationToken::IsCancellationRequested() const noexcept {
    return state_ && state_->cancelled.load(std::memory_order_acquire);
}

CancellationSource::CancellationSource()
    : state_(std::make_shared<CancellationToken::State>()) {}

CancellationToken CancellationSource::Token() const {
    return CancellationToken(state_);
}

void CancellationSource::Cancel() noexcept {
    state_->cancelled.store(true, std::memory_order_release);
}

bool CancellationSource::IsCancellationRequested() const noexcept {
    return state_->cancelled.load(std::memory_order_acquire);
}

} // namespace cpptoolkit::concurrency
