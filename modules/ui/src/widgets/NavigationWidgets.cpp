#include "cpptoolkit/ui/widgets/NavigationWidgets.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace cpptoolkit::ui {

TabBarWidget::TabBarWidget(std::string label, mvvm::ObservableProperty<int>& selectedTab)
    : label_(std::move(label)), selectedTab_(selectedTab) {}

Panel& TabBarWidget::AddTab(std::string label) {
    if (tabs_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("TabBarWidget has too many tabs");
    tabs_.push_back({std::move(label), std::make_unique<Panel>()});
    return *tabs_.back().Content;
}

void TabBarWidget::Draw() {
    if (!Visible) return;
    const int selected = selectedTab_.Get();
    if (selected < 0 || static_cast<std::size_t>(selected) >= tabs_.size()) {
        ImGui::TextUnformatted("Invalid selected tab index");
        return;
    }
    ImGui::PushID(this);
    if (ImGui::BeginTabBar(label_.c_str())) {
        const bool programmatic = selected != lastSelection_;
        for (std::size_t i = 0; i < tabs_.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            const auto flags = programmatic && selected == static_cast<int>(i) ?
                               ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            const bool active = ImGui::BeginTabItem(tabs_[i].Label.c_str(), nullptr, flags);
            if (active) {
                if (!programmatic && selected != static_cast<int>(i)) {
                    selectedTab_.Set(static_cast<int>(i));
                    if (OnSelectionChanged) OnSelectionChanged(static_cast<int>(i));
                }
                tabs_[i].Content->Draw();
                ImGui::EndTabItem();
            }
            ImGui::PopID();
        }
        lastSelection_ = selectedTab_.Get();
        ImGui::EndTabBar();
    }
    ImGui::PopID();
}

void TabBarWidget::Build() {
    OnBuild();
    for (auto& tab : tabs_) {
        if (!tab.Content->HasViewModel())
            tab.Content->SetViewModel(GetViewModel<mvvm::ObservableObject>());
        if (!tab.Content->HasApplication()) tab.Content->SetApplication(GetApplication());
        tab.Content->Build();
    }
    BuildChildren();
}

void TabBarWidget::Destroy() {
    for (auto& tab : tabs_) tab.Content->Destroy();
    tabs_.clear();
    DestroyChildren();
}

MenuBarWidget::MenuBarWidget(bool mainMenu) : mainMenu_(mainMenu) {}

void MenuBarWidget::Draw() {
    if (!Visible) return;
    const bool open = mainMenu_ ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();
    if (open) {
        DrawChildren();
        if (mainMenu_) ImGui::EndMainMenuBar();
        else ImGui::EndMenuBar();
    }
}

MenuWidget::MenuWidget(std::string label) : label_(std::move(label)) {}

void MenuWidget::Draw() {
    if (!Visible) return;
    if (ImGui::BeginMenu(label_.c_str(), Enabled)) {
        DrawChildren();
        ImGui::EndMenu();
    }
}

MenuItemWidget::MenuItemWidget(std::string label, mvvm::Command& command, std::string shortcut)
    : label_(std::move(label)), command_(command), shortcut_(std::move(shortcut)) {}

void MenuItemWidget::Draw() {
    if (ImGui::MenuItem(label_.c_str(), shortcut_.c_str(), false, command_.CanExecute())) {
        command_.Execute();
        if (OnClick) OnClick();
    }
}

ToolbarWidget::ToolbarWidget() : StackPanel(Orientation::Horizontal) {}

BreadcrumbWidget::BreadcrumbWidget(std::vector<std::string> segments,
                                   mvvm::ObservableProperty<int>& selectedSegment,
                                   std::function<void(int)> onSelectionChanged)
    : OnSelectionChanged(std::move(onSelectionChanged)), segments_(std::move(segments)),
      selectedSegment_(selectedSegment) {
    if (segments_.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("BreadcrumbWidget has too many segments");
}

void BreadcrumbWidget::Draw() {
    ImGui::PushID(this);
    for (std::size_t i = 0; i < segments_.size(); ++i) {
        if (i != 0) {
            ImGui::SameLine();
            ImGui::TextUnformatted("/");
            ImGui::SameLine();
        }
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::SmallButton(segments_[i].c_str()) && selectedSegment_.Get() != static_cast<int>(i)) {
            selectedSegment_.Set(static_cast<int>(i));
            if (OnSelectionChanged) OnSelectionChanged(static_cast<int>(i));
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

} // namespace cpptoolkit::ui
