#pragma once

/** @file
 *  @brief Tooltips, property-controlled popups, modal dialogs and context menus.
 */

#include <functional>
#include <string>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Container shown when the previously submitted ImGui item is hovered.
 *  @note Insert directly after the target widget; its children supply tooltip content.
 */
class TooltipWidget : public Widget {
public:
    /** @brief Draw child contents inside an ImGui item tooltip when active. */
    void Draw() override;
};

/** @brief Popup presentation and activation policy. */
enum class PopupKind {
    Popup,      ///< Nonmodal popup opened by its bound property.
    Modal,      ///< Modal dialog with a title-bar close button.
    ContextMenu ///< Right-click popup attached to the previous item; also property-openable.
};

/** @brief Popup container synchronizing a borrowed visibility flag.
 *
 * Draw() must run under a stable ImGui ID/window scope. The visibility property
 * must outlive the widget. External changes open/close without event callbacks.
 * User context-menu openings set the flag before OnOpened; user dismissals and
 * close commands invoked by child controls set it false before OnClosed.
 * Keep this widget in a continuously drawn scope if it can be opened from
 * controls outside its current tab or conditional container.
 */
class PopupWidget : public Widget {
public:
    /** @brief Bind popup visibility.
     *  @param label Popup ID and, for modals, window title; use unique modal titles.
     *  @param open Borrowed visibility flag.
     *  @param kind Presentation and activation policy.
     */
    PopupWidget(std::string label, mvvm::ObservableProperty<bool>& open,
                PopupKind kind = PopupKind::Popup);
    /** @brief Synchronize visibility, render contents and process user transitions. */
    void Draw() override;
    /** @brief Render-thread callback after a user opening commits open=true. */
    std::function<void()> OnOpened;
    /** @brief Render-thread callback after a user closing commits open=false. */
    std::function<void()> OnClosed;

private:
    std::string label_;
    mvvm::ObservableProperty<bool>& open_;
    PopupKind kind_;
    bool wasOpen_ = false;
    bool lastRequested_ = false;
};

/** @brief Modal PopupWidget specialization blocking interaction behind its window. */
class DialogWidget : public PopupWidget {
public:
    /** @brief Bind a modal dialog.
     *  @param label Unique dialog ID and title.
     *  @param open Borrowed visibility flag which must outlive the dialog.
     */
    DialogWidget(std::string label, mvvm::ObservableProperty<bool>& open);
};

/** @brief PopupWidget specialization activated by right-clicking the previous item. */
class ContextMenuWidget : public PopupWidget {
public:
    /** @brief Bind a context menu.
     *  @param label Popup ID.
     *  @param open Borrowed visibility flag which must outlive the menu.
     */
    ContextMenuWidget(std::string label, mvvm::ObservableProperty<bool>& open);
};

} // namespace cpptoolkit::ui
