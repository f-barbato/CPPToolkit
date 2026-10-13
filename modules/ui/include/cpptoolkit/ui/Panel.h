#pragma once

/** @file
 *  @brief Inline retained containers and optional titled ImGui windows.
 */

#include <string>
#include <imgui.h>
#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Inline container by default, or a standalone window when titled.
 *  @note Default construction preserves tab, splitter and gallery content behavior.
 */
class Panel : public Widget {
public:
    /** @brief Construct an inline child container. */
    Panel() = default;
    /** @brief Construct a window panel.
     *  @param title Window title and ImGui ID.
     */
    explicit Panel(const std::string& title) : title_(title), window_(true) {}
    /** @brief Render inline contents or an open, titled ImGui window. */
    void Draw() override {
        if (!Visible) return;
        if (!window_) {
            DrawChildren();
            return;
        }
        if (!open_) return;
        if (ImGui::Begin(title_.c_str(), closable_ ? &open_ : nullptr, flags_)) DrawChildren();
        ImGui::End();
    }
    /** @brief Show or hide the window close button (also the docked-tab close button).
     *  @param closable False removes the X so the user cannot close the window; default true.
     *  @note SetOpen() still closes the window programmatically.
     */
    void SetClosable(bool closable) { closable_ = closable; }
    /** @brief Query whether the window shows a close button.
     *  @return True if closable.
     */
    bool IsClosable() const { return closable_; }
    /** @brief Add window flags.
     *  @param flags Flags to enable.
     */
    void SetFlags(ImGuiWindowFlags flags) { flags_ |= flags; }
    /** @brief Remove window flags.
     *  @param flags Flags to disable.
     */
    void ClearFlags(ImGuiWindowFlags flags) { flags_ &= ~flags; }
    /** @brief Set window visibility.
     *  @param open Whether a titled window is open.
     */
    void SetOpen(bool open) { open_ = open; }
    /** @brief Query window visibility.
     *  @return Current open flag.
     */
    bool IsOpen() const { return open_; }
    /** @brief Set the title and switch this container to window mode.
     *  @param title Window title and ID.
     */
    void SetTitle(const std::string& title) { title_ = title; window_ = true; }
    /** @brief Access the window title.
     *  @return Stored title, empty for a default inline panel.
     */
    const std::string& GetTitle() const { return title_; }
    /** @brief Query window flags.
     *  @return Enabled flags.
     */
    ImGuiWindowFlags GetFlags() const { return flags_; }
    /** @brief Test whether any of the specified flags are enabled.
     *  @param flag Flag or mask.
     *  @return True if any specified bit is enabled.
     */
    bool HasFlag(ImGuiWindowFlags flag) const { return (flags_ & flag) != 0; }

private:
    ImGuiWindowFlags flags_ = 0;
    bool open_ = true;
    bool closable_ = true;
    std::string title_;
    bool window_ = false;
};

} // namespace cpptoolkit::ui
