#pragma once
#include <cpptoolkit/ui/Export.h>

/** @file
 *  @brief Generic keyboard shortcut registry.
 */

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>

#include "cpptoolkit/mvvm/Command.h"

namespace cpptoolkit::ui {

/** @brief Registration data for one shortcut. */
struct ShortcutInfo {
    /** @brief Unique stable identifier (e.g. "file.save"). */
    std::string Id;
    /** @brief Human-readable description for menus and help screens. */
    std::string Description;
    /** @brief Key plus optional modifiers, e.g. ImGuiMod_Ctrl | ImGuiKey_S. */
    ImGuiKeyChord Chord = ImGuiKey_None;
    /** @brief Action invoked on the render thread when the chord is pressed. */
    std::function<void()> Action;
    /** @brief Optional predicate; the shortcut is skipped while it returns false. */
    std::function<bool()> CanExecute;
    /** @brief Fire repeatedly while the chord is held. */
    bool Repeat = false;
    /** @brief Allow chords without Ctrl/Alt/Super to fire while a text field is being edited. */
    bool AllowWhileTyping = false;
};

/**
 * @brief Registry mapping key chords to actions, processed once per frame.
 *
 * Chords are unique: registering or rebinding onto a chord already in use is
 * rejected. Application owns an instance and calls Process() each frame; it
 * can also be driven manually inside an ImGui frame. Render-thread-only.
 * Actions may register, rebind or unregister shortcuts.
 */
class CPPTOOLKIT_UI_EXPORT ShortcutManager {
public:
    /**
     * @brief Add a shortcut.
     * @param info Shortcut definition; Id must be non-empty and unique, Chord non-zero and unused, Action non-empty.
     * @throws std::invalid_argument On an invalid definition, duplicate id or chord conflict.
     */
    void Register(ShortcutInfo info);
    /**
     * @brief Add a shortcut invoking a borrowed command, gated by its CanExecute().
     * @param id Unique identifier.
     * @param chord Key chord.
     * @param command Command that must outlive the registration.
     * @param description Optional description.
     * @throws std::invalid_argument As Register(ShortcutInfo).
     */
    void Register(std::string id, ImGuiKeyChord chord, mvvm::Command& command, std::string description = {});
    /**
     * @brief Remove a shortcut.
     * @param id Identifier.
     * @return True if it existed.
     */
    bool Unregister(const std::string& id);
    /**
     * @brief Change the chord of a shortcut.
     * @param id Identifier.
     * @param chord New chord.
     * @return False if the id is unknown or the chord is used by another shortcut.
     */
    bool Rebind(const std::string& id, ImGuiKeyChord chord);
    /**
     * @brief Look up a shortcut.
     * @param id Identifier.
     * @return Pointer valid until the registry changes, or nullptr.
     */
    [[nodiscard]] const ShortcutInfo* Find(const std::string& id) const;
    /**
     * @brief Find the shortcut using a chord.
     * @param chord Key chord.
     * @return Matching shortcut or nullptr.
     */
    [[nodiscard]] const ShortcutInfo* FindByChord(ImGuiKeyChord chord) const;
    /**
     * @brief Display text of a shortcut, e.g. "Ctrl+S".
     * @param id Identifier.
     * @return Chord text, or empty if unknown.
     */
    [[nodiscard]] std::string Text(const std::string& id) const;
    /** @brief All registered shortcuts in registration order.
     *  @return Registered shortcuts.
     */
    [[nodiscard]] const std::vector<ShortcutInfo>& All() const { return items_; }
    /**
     * @brief Invoke the actions of shortcuts pressed in the current ImGui frame.
     * @note Skipped entirely while Enabled is false. Uses global routing, so focused
     *       items (e.g. text-field Ctrl+Z) keep priority over matching shortcuts.
     */
    void Process();
    /** @brief Suspend all shortcuts (e.g. while capturing a new chord). */
    bool Enabled = true;

private:
    std::vector<ShortcutInfo> items_;
};

} // namespace cpptoolkit::ui
