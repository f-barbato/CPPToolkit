#pragma once

/** @file
 *  @brief Views with shared ownership of a typed MVVM ViewModel.
 */

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/mvvm/ObservableObject.h"

namespace cpptoolkit::ui {

    /** @brief Common observable base for view models. */
    typedef mvvm::ObservableObject ViewModel;
    /** @brief Shared ownership of an untyped view model. */
    typedef std::shared_ptr<ViewModel> ViewModelPtr;

    /** @brief Widget root retaining a shared, typed ViewModel.
     *  @tparam T ObservableObject-derived model type.
     *  @tparam Args Constructor argument types for the model.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ViewModel, T>>, typename... Args>
    class View : public Widget {
    public:
        /** @brief Construct the owned model.
         *  @param args Arguments forwarded to T's constructor.
         */
        View(Args&&... args){
            this->_viewModel = std::make_shared<T>(std::forward<Args>(args)...);
            this->SetViewModel(this->_viewModel.get());
        }
        /** @brief Destroy the view and release its shared model reference. */
        virtual ~View() = default;

    protected:
        friend class Application;
        /** @brief Model shared by this view; must outlive widgets borrowing its properties. */
        std::shared_ptr<T> _viewModel;
    };

    /** @brief Shared pointer alias for a typed View.
     *  @tparam T ObservableObject-derived view model.
     *  @tparam Args Additional View template arguments.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ViewModel, T>>, typename... Args>
    using ViewPtr = std::shared_ptr<View<T, Args...>>;

} // namespace cpptoolkit::ui