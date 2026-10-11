#include <cpptoolkit/concurrency/concurrency.h>

#include <gtest/gtest.h>

#include <future>
#include <thread>
#include <vector>

using namespace cpptoolkit::concurrency;

TEST(WorkQueueTests, DispatcherDrainsSnapshotInOrderOnCallingThread) {
    Dispatcher dispatcher;
    std::vector<int> calls;
    const auto thread = std::this_thread::get_id();
    dispatcher.Post([&] {
        EXPECT_EQ(std::this_thread::get_id(), thread);
        calls.push_back(1);
        dispatcher.Post([&] { calls.push_back(3); });
    });
    dispatcher.Post([&] { calls.push_back(2); });
    dispatcher.ProcessPending();
    EXPECT_EQ(calls, (std::vector<int>{1, 2}));
    dispatcher.ProcessPending();
    EXPECT_EQ(calls, (std::vector<int>{1, 2, 3}));
}

TEST(WorkQueueTests, WorkerExecutesFifoOnBackgroundThread) {
    std::vector<int> calls;
    std::promise<std::thread::id> finished;
    auto result = finished.get_future();
    auto& worker = NotificationWorker::Instance();
    worker.Post([&] { calls.push_back(1); });
    worker.Post([&] {
        calls.push_back(2);
        finished.set_value(std::this_thread::get_id());
    });
    result.wait();
    EXPECT_NE(result.get(), std::this_thread::get_id());
    EXPECT_EQ(calls, (std::vector<int>{1, 2}));
}
