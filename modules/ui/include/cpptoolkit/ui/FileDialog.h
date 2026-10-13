#pragma once

/** @file
 *  @brief Operating-system file open/save/folder dialogs.
 */

#include <cpptoolkit/ui/Export.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cpptoolkit::ui {

/** @brief Named set of file extensions offered by a dialog. */
struct FileFilter {
    /** @brief Display name, e.g. "Images". */
    std::string name;
    /** @brief Comma-separated extensions without dots, e.g. "png,jpg"; "*" is not supported. */
    std::string extensions;
};

/** @brief Outcome of an asynchronous dialog. */
struct FileDialogResult {
    /** @brief Selected paths (UTF-8); empty if cancelled or failed. */
    std::vector<std::string> paths;
    /** @brief Failure description; empty on success or cancellation. */
    std::string error;
};

/** @brief Handle to a dialog running off the render thread; poll IsReady() each frame. */
class CPPTOOLKIT_UI_EXPORT PendingFileDialog {
public:
    /** @brief Opaque shared state. */
    struct State;

    /** @brief Create an empty handle (Valid() is false). */
    PendingFileDialog();
    /** @brief Wrap shared state; used internally by FileDialog.
     *  @param state Shared dialog state.
     */
    explicit PendingFileDialog(std::shared_ptr<State> state);
    /** @brief Release the handle; a running dialog is detached. */
    ~PendingFileDialog();
    /** @brief Move-construct a handle.
     *  @param other Source, left invalid.
     */
    PendingFileDialog(PendingFileDialog&& other) noexcept;
    /** @brief Move-assign a handle.
     *  @param other Source, left invalid.
     *  @return This handle.
     */
    PendingFileDialog& operator=(PendingFileDialog&& other) noexcept;
    PendingFileDialog(const PendingFileDialog&) = delete;
    PendingFileDialog& operator=(const PendingFileDialog&) = delete;

    /** @brief True while a dialog is outstanding (running or not yet taken).
     *  @return Whether a dialog is outstanding.
     */
    bool Valid() const;
    /** @brief True once the dialog closed and the result can be taken. Thread-safe.
     *  @return Whether Take() can be called.
     */
    bool IsReady() const;
    /** @brief Retrieve the result and invalidate the handle.
     *  @return Dialog outcome.
     *  @throws std::logic_error If not ready.
     */
    FileDialogResult Take();

private:
    std::shared_ptr<State> state_;
};

/** @brief Show native modal dialogs (via nativefiledialog-extended).
 *
 *  The synchronous functions block the calling thread until the dialog closes;
 *  call them from the render thread (macOS requires the main thread). The
 *  `*Async` variants run the dialog on a worker thread so rendering continues;
 *  on macOS, where Cocoa needs the main thread, they complete synchronously.
 *  The parent window is captured on the calling (render) thread, so call the
 *  async functions from there. Paths are UTF-8. A cancelled dialog yields an
 *  empty result; a platform failure throws (sync) or fills the error (async).
 */
class CPPTOOLKIT_UI_EXPORT FileDialog {
public:
    /** @brief Asynchronous OpenFile().
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @return Handle to poll for the result.
     */
    static PendingFileDialog OpenFileAsync(const std::vector<FileFilter>& filters = {},
                                           const std::string& defaultPath = {});
    /** @brief Asynchronous OpenFiles().
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @return Handle to poll for the result.
     */
    static PendingFileDialog OpenFilesAsync(const std::vector<FileFilter>& filters = {},
                                            const std::string& defaultPath = {});
    /** @brief Asynchronous SaveFile().
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @param defaultName Optional suggested file name.
     *  @return Handle to poll for the result.
     */
    static PendingFileDialog SaveFileAsync(const std::vector<FileFilter>& filters = {},
                                           const std::string& defaultPath = {},
                                           const std::string& defaultName = {});
    /** @brief Asynchronous PickFolder().
     *  @param defaultPath Optional initial directory.
     *  @return Handle to poll for the result.
     */
    static PendingFileDialog PickFolderAsync(const std::string& defaultPath = {});

    /** @brief Choose one existing file.
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @return Selected path, or nullopt if cancelled.
     *  @throws std::runtime_error If the dialog cannot be shown.
     */
    static std::optional<std::string> OpenFile(const std::vector<FileFilter>& filters = {},
                                               const std::string& defaultPath = {});
    /** @brief Choose several existing files.
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @return Selected paths; empty if cancelled.
     *  @throws std::runtime_error If the dialog cannot be shown.
     */
    static std::vector<std::string> OpenFiles(const std::vector<FileFilter>& filters = {},
                                              const std::string& defaultPath = {});
    /** @brief Choose a destination file, with overwrite confirmation by the OS.
     *  @param filters Optional extension filters.
     *  @param defaultPath Optional initial directory.
     *  @param defaultName Optional suggested file name.
     *  @return Selected path, or nullopt if cancelled.
     *  @throws std::runtime_error If the dialog cannot be shown.
     */
    static std::optional<std::string> SaveFile(const std::vector<FileFilter>& filters = {},
                                               const std::string& defaultPath = {},
                                               const std::string& defaultName = {});
    /** @brief Choose a folder.
     *  @param defaultPath Optional initial directory.
     *  @return Selected folder, or nullopt if cancelled.
     *  @throws std::runtime_error If the dialog cannot be shown.
     */
    static std::optional<std::string> PickFolder(const std::string& defaultPath = {});
};

} // namespace cpptoolkit::ui
