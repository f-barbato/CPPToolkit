#pragma once

#include <memory>
#include <string>
#include <thread>

#include <cpptoolkit/concurrency/concurrency.h>
#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/widgets/widgets.h>

/** @brief ViewModel running a cancellable simulated task on an owned background thread. */
class CancellationExampleState : public cpptoolkit::mvvm::ObservableObject {
public:
    cpptoolkit::mvvm::ObservableProperty<bool> Running{this, "Running", false};
    cpptoolkit::mvvm::ObservableProperty<float> Progress{this, "Progress", 0};
    cpptoolkit::mvvm::ObservableProperty<std::string> Status{this, "Status", "Ready"};

    cpptoolkit::mvvm::Command Start{[this] { StartTask(); }, [this] { return !Running.Get(); }};
    cpptoolkit::mvvm::Command Cancel{[this] { cancellation_->Cancel(); }, [this] { return Running.Get(); }};

    ~CancellationExampleState() override;

    /** @brief Request cancellation and join the worker thread. */
    void StopAndJoin();

private:
    void StartTask();

    std::unique_ptr<cpptoolkit::concurrency::CancellationSource> cancellation_;
    std::thread worker_;
};

/** @brief Panel with Start/Cancel buttons, progress and status bound to a CancellationExampleState. */
class CancellationExampleWidget : public cpptoolkit::ui::Panel {
public:
    CancellationExampleState State;

    void Destroy() override;

protected:
    void OnBuild() override;
};
