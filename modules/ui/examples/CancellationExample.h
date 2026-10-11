#pragma once

#include <chrono>
#include <memory>
#include <system_error>
#include <thread>

#include <cpptoolkit/concurrency/concurrency.h>
#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/widgets/widgets.h>

class CancellationExampleState : public cpptoolkit::mvvm::ObservableObject {
public:
    cpptoolkit::mvvm::ObservableProperty<bool> Running{this, "Running", false};
    cpptoolkit::mvvm::ObservableProperty<float> Progress{this, "Progress", 0};
    cpptoolkit::mvvm::ObservableProperty<std::string> Status{this, "Status", "Ready"};

    cpptoolkit::mvvm::Command Start{[this] { StartTask(); },
        [this] { return !Running.Get(); }};
    cpptoolkit::mvvm::Command Cancel{[this] {
        cancellation_->Cancel();
    }, [this] { return Running.Get(); }};

    ~CancellationExampleState() override { StopAndJoin(); }

    void StopAndJoin() {
        if (cancellation_) cancellation_->Cancel();
        if (worker_.joinable()) worker_.join();
    }

private:
    void StartTask() {
        if (Running.Get()) return;
        if (worker_.joinable()) worker_.join();
        // A cancelled source cannot be reset: each run gets a fresh source.
        cancellation_ = std::make_unique<cpptoolkit::concurrency::CancellationSource>();
        Progress.Set(0);
        Status.Set("Running in background");
        Running.Set(true);
        try {
            worker_ = std::thread([this, token = cancellation_->Token()] {
                for (int step = 1; step <= 100; ++step) {
                    // Simulate bounded chunks of work without touching UI/GPU APIs.
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    if (token.IsCancellationRequested()) {
                        Status.Set("Cancelled");
                        Running.Set(false);
                        return;
                    }
                    Progress.Set(static_cast<float>(step) / 100.0f);
                }
                Status.Set("Completed");
                Running.Set(false);
            });
        } catch (const std::system_error& error) {
            Status.Set(std::string("Failed to start task: ") + error.what());
            Running.Set(false);
            throw;
        }
    }

    std::unique_ptr<cpptoolkit::concurrency::CancellationSource> cancellation_;
    std::thread worker_;
};

class CancellationExampleWidget : public cpptoolkit::ui::Panel {
public:
    CancellationExampleState State;

    void Destroy() override {
        State.StopAndJoin();
        Panel::Destroy();
    }

protected:
    void OnBuild() override {
        using namespace cpptoolkit;
        Add<ui::TextWidget>("Cooperative cancellation (dedicated background thread)");
        Add<ui::ButtonWidget>("Start task", State.Start);
        Add<ui::ButtonWidget>("Cancel task", State.Cancel);
        Add<ui::ProgressBarWidget>("Task progress", State.Progress);
        Add<ui::TextWidget>(State.Status);
        Add<ui::TextWidget>("Cancel requests a stop; the task checks its token after each chunk.");
    }
};
