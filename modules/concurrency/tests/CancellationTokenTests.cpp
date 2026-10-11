#include <cpptoolkit/concurrency/concurrency.h>

#include <gtest/gtest.h>

#include <future>
#include <memory>
#include <thread>

using cpptoolkit::concurrency::CancellationSource;
using cpptoolkit::concurrency::CancellationToken;

TEST(CancellationTokenTests, DefaultTokenCannotBeCancelled) {
    const CancellationToken token;

    EXPECT_FALSE(token.IsCancellationRequested());
}

TEST(CancellationTokenTests, CopiesObserveCancellationFromSource) {
    CancellationSource source;
    const auto token = source.Token();
    const auto tokenCopy = token;

    EXPECT_FALSE(source.IsCancellationRequested());
    EXPECT_FALSE(token.IsCancellationRequested());

    source.Cancel();

    EXPECT_TRUE(source.IsCancellationRequested());
    EXPECT_TRUE(token.IsCancellationRequested());
    EXPECT_TRUE(tokenCopy.IsCancellationRequested());
}

TEST(CancellationTokenTests, TokenRemainsValidAfterSourceDestruction) {
    CancellationToken token;
    {
        auto source = std::make_unique<CancellationSource>();
        token = source->Token();
        source->Cancel();
    }

    EXPECT_TRUE(token.IsCancellationRequested());
}

TEST(CancellationTokenTests, CancellationIsIdempotentAndVisibleToNewTokens) {
    CancellationSource source;
    source.Cancel();
    source.Cancel();

    EXPECT_TRUE(source.Token().IsCancellationRequested());
}

TEST(CancellationTokenTests, SourcesHaveIndependentState) {
    CancellationSource first;
    CancellationSource second;
    first.Cancel();

    EXPECT_TRUE(first.Token().IsCancellationRequested());
    EXPECT_FALSE(second.Token().IsCancellationRequested());
}

TEST(CancellationTokenTests, DestroyingSourceDoesNotRequestCancellation) {
    CancellationToken token;
    {
        CancellationSource source;
        token = source.Token();
    }

    EXPECT_FALSE(token.IsCancellationRequested());
}

TEST(CancellationTokenTests, CancellationIsObservedOnAnotherThread) {
    CancellationSource source;
    const auto token = source.Token();
    std::promise<void> cancelled;
    auto ready = cancelled.get_future();
    std::thread worker([&source, &cancelled] {
        source.Cancel();
        cancelled.set_value();
    });
    ready.wait();

    EXPECT_TRUE(token.IsCancellationRequested());
    worker.join();
}
