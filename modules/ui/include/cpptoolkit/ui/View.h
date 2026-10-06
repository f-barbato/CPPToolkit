#pragma once

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/mvvm/ObservableObject.h"

namespace cpptoolkit::ui {

    typedef mvvm::ObservableObject ViewModel;
    typedef std::shared_ptr<ViewModel> ViewModelPtr;

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ViewModel, T>>, typename... Args>
    class View : public Widget {
    public:
        View(Args&&... args){ this->_viewModel = std::make_shared<T>(std::forward<Args>(args)...); }
        virtual ~View() = default;

    protected:
        friend class Application;
        std::shared_ptr<T> _viewModel;
    };

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ViewModel, T>>, typename... Args>
    using ViewPtr = std::shared_ptr<View<T, Args...>>;

} // namespace cpptoolkit::ui