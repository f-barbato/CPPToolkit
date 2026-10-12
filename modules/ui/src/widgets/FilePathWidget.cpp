#include "cpptoolkit/ui/widgets/FilePathWidget.h"

#include <exception>
#include <optional>
#include <utility>

#include "../../detail/TextInput.h"

namespace cpptoolkit::ui {

FilePathWidget::FilePathWidget(std::string label, mvvm::ObservableProperty<std::string>& bound, FilePathMode mode,
                               std::vector<FileFilter> filters, PathChangedHandler onPathChanged)
    : OnPathChanged(std::move(onPathChanged)), label_(std::move(label)), bound_(bound), mode_(mode),
      filters_(std::move(filters)) {}

void FilePathWidget::Draw() {
    ImGui::PushID(this);
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float buttonWidth = ImGui::CalcTextSize("Browse...").x + ImGui::GetStyle().FramePadding.x * 2;
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth - spacing);
    detail::DrawText("##path", bound_, buffer_, readOnly, detail::TextMode::SingleLine, OnPathChanged);
    ImGui::SameLine(0, spacing);
    if (pending_.IsReady()) {
        FileDialogResult result = pending_.Take();
        lastError_ = std::move(result.error);
        if (!result.paths.empty() && result.paths.front() != bound_.Get()) {
            bound_.Set(result.paths.front());
            if (OnPathChanged) OnPathChanged(result.paths.front());
        }
    }
    ImGui::BeginDisabled(readOnly || pending_.Valid());
    if (ImGui::Button("Browse...")) {
        switch (mode_) {
        case FilePathMode::Open: pending_ = FileDialog::OpenFileAsync(filters_); break;
        case FilePathMode::Save: pending_ = FileDialog::SaveFileAsync(filters_); break;
        case FilePathMode::Folder: pending_ = FileDialog::PickFolderAsync(); break;
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine(0, spacing);
    ImGui::TextUnformatted(label_.c_str());
    ImGui::PopID();
}

} // namespace cpptoolkit::ui
