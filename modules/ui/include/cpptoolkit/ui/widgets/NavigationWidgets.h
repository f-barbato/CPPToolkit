#pragma once

/** @file
 *  @brief Retained tabs, menus, toolbars and breadcrumbs.
 */

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <cpptoolkit/mvvm/Command.h>
#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/widgets/LayoutWidgets.h"

namespace cpptoolkit::ui {

/** @brief Tabbed container with a borrowed selected-tab index.
 *
 * The property must outlive the widget. AddTab() owns stable Panel instances.
 * A valid selection is in [0,tab-count); an invalid index displays a diagnostic.
 * Programmatic selection changes are applied without user callbacks.
 * Populate AddTab() panels rather than inherited direct children.
 */
class TabBarWidget : public Widget {
public:
    /** @brief Bind tab selection.
     *  @param label ImGui tab-bar ID.
     *  @param selectedTab Borrowed zero-based active-tab index.
     */
    TabBarWidget(std::string label, mvvm::ObservableProperty<int>& selectedTab);
    /** @brief Add an owned tab content panel.
     *  @param label Tab heading.
     *  @return Panel reference valid until Destroy() or widget destruction.
     *  @throws std::length_error If the tab count would exceed int indexing.
     */
    Panel& AddTab(std::string label);
    /** @brief Render tabs and draw the active content panel. */
    void Draw() override;
    /** @brief Build every tab panel, including inactive tabs. */
    void Build() override;
    /** @brief Tear down and release every tab and inherited direct child. */
    void Destroy() override;
    /** @brief User-tab-selection callback after property commit on the render thread. */
    std::function<void(int)> OnSelectionChanged;

private:
    struct Tab {
        std::string Label;
        std::unique_ptr<Panel> Content;
    };
    std::string label_;
    mvvm::ObservableProperty<int>& selectedTab_;
    std::vector<Tab> tabs_;
    int lastSelection_ = -2;
};

/** @brief Container for MenuWidget children in a main or window menu bar.
 *  @note Window menu bars require ImGuiWindowFlags_MenuBar on their host window.
 */
class MenuBarWidget : public Widget {
public:
    /** @brief Choose menu-bar placement.
     *  @param mainMenu True for the application-wide main menu bar.
     */
    explicit MenuBarWidget(bool mainMenu = false);
    /** @brief Draw menu children when the menu bar can be opened. */
    void Draw() override;

private:
    bool mainMenu_;
};

/** @brief Menu or submenu owning its item widgets. */
class MenuWidget : public Widget {
public:
    /** @brief Set the menu heading.
     *  @param label ImGui menu heading and ID.
     */
    explicit MenuWidget(std::string label);
    /** @brief Draw items while this menu is open. */
    void Draw() override;
    /** @brief Whether the menu can be opened by the user. */
    bool Enabled = true;

private:
    std::string label_;
};

/** @brief Menu action bound to a borrowed Command.
 *  @note The command must outlive the widget. CanExecute() controls enabled state.
 */
class MenuItemWidget : public Widget {
public:
    /** @brief Bind a menu item.
     *  @param label Item heading and ID.
     *  @param command Borrowed action.
     *  @param shortcut Display-only shortcut hint; no keyboard handling is installed.
     */
    MenuItemWidget(std::string label, mvvm::Command& command, std::string shortcut = "");
    /** @brief Render the enabled state and execute activated items. */
    void Draw() override;
    /** @brief Render-thread callback after the activated command executes. */
    std::function<void()> OnClick;

private:
    std::string label_;
    mvvm::Command& command_;
    std::string shortcut_;
};

/** @brief Horizontal StackPanel intended to contain command buttons and controls. */
class ToolbarWidget : public StackPanel {
public:
    /** @brief Construct a horizontal toolbar. */
    ToolbarWidget();
};

/** @brief Fixed breadcrumb path with a borrowed selected-segment index.
 *  @note Clicks update selection but do not truncate the stored path.
 */
class BreadcrumbWidget : public Widget {
public:
    /** @brief Bind breadcrumb navigation.
     *  @param segments Fixed path labels copied/moved into the widget.
     *  @param selectedSegment Borrowed zero-based index which must outlive the widget.
     *  @param onSelectionChanged Optional user-navigation callback.
     *  @throws std::length_error If segment count exceeds int indexing.
     */
    BreadcrumbWidget(std::vector<std::string> segments,
                     mvvm::ObservableProperty<int>& selectedSegment,
                     std::function<void(int)> onSelectionChanged = {});
    /** @brief Render path buttons and commit new user selections. */
    void Draw() override;
    /** @brief Render-thread callback after committing a different segment index. */
    std::function<void(int)> OnSelectionChanged;

private:
    std::vector<std::string> segments_;
    mvvm::ObservableProperty<int>& selectedSegment_;
};

} // namespace cpptoolkit::ui
