#pragma once

/** @file
 *  @brief Path text box with a native file/folder browse button.
 */

#include <functional>
#include <string>
#include <vector>

#include <cpptoolkit/mvvm/ObservableProperty.h>

#include "cpptoolkit/ui/FileDialog.h"
#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Kind of path selected by FilePathWidget. */
enum class FilePathMode {
    Open,   ///< Existing file.
    Save,   ///< Destination file.
    Folder  ///< Existing folder.
};

/** @brief Edit a borrowed UTF-8 path property, optionally filled by a native dialog.
 *
 * The property must outlive this widget. Typing or choosing a path commits the
 * property before OnPathChanged executes on the render thread. Programmatic
 * changes are polled without triggering the callback.  The dialog runs on a
  * worker thread (synchronously on macOS) and is polled each frame, so rendering
  * continues while it is open; a cancelled dialog leaves the property unchanged.
 * Dialog failures are reported through LastError().
 */
class CPPTOOLKIT_UI_EXPORT FilePathWidget : public Widget {
public:
    /** @brief Callback receiving the new path after property commit. */
    using PathChangedHandler = std::function<void(const std::string&)>;

    /** @brief Construct the path binding.
     *  @param label ImGui label and item ID.
     *  @param bound Borrowed path property.
     *  @param mode Kind of path chosen by the browse button.
     *  @param filters Extension filters for Open/Save dialogs.
     *  @param onPathChanged Optional user-edit handler.
     */
    FilePathWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                   FilePathMode mode = FilePathMode::Open, std::vector<FileFilter> filters = {},
                   PathChangedHandler onPathChanged = {});
    /** @brief Poll the path and submit the text input and browse button. */
    void Draw() override;
    /** @brief Query the last dialog failure.
     *  @return Error text, empty after a successful or cancelled dialog.
     */
    [[nodiscard]] const std::string& LastError() const { return lastError_; }

public:
    /** @brief Optional render-thread handler invoked after user edits or a chosen path. */
    PathChangedHandler OnPathChanged;
    /** @brief Disable editing and browsing while continuing to poll the bound value. */
    bool readOnly = false;

private:
    std::string label_;
    mvvm::ObservableProperty<std::string>& bound_;
    FilePathMode mode_;
    std::vector<FileFilter> filters_;
    std::vector<char> buffer_;
    std::string lastError_;
    PendingFileDialog pending_;
};

} // namespace cpptoolkit::ui
