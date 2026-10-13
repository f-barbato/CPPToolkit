#include <cpptoolkit/ui/Shortcuts.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <imgui_internal.h>

namespace cpptoolkit::ui {
namespace {

bool HasCommandModifier(ImGuiKeyChord chord) {
    return (chord & (ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiMod_Super)) != 0;
}

} // namespace

void ShortcutManager::Register(ShortcutInfo info) {
    if (info.Id.empty()) throw std::invalid_argument("Shortcut id must not be empty");
    if (info.Chord == ImGuiKey_None) throw std::invalid_argument("Shortcut chord must not be empty");
    if (!info.Action) throw std::invalid_argument("Shortcut action must not be empty");
    if (Find(info.Id)) throw std::invalid_argument("Duplicate shortcut id: " + info.Id);
    if (const auto* other = FindByChord(info.Chord)) {
        throw std::invalid_argument("Shortcut chord already used by: " + other->Id);
    }
    items_.push_back(std::move(info));
}

void ShortcutManager::Register(std::string id, ImGuiKeyChord chord, mvvm::Command& command,
                               std::string description) {
    ShortcutInfo info;
    info.Id = std::move(id);
    info.Description = std::move(description);
    info.Chord = chord;
    info.Action = [&command] { command.Execute(); };
    info.CanExecute = [&command] { return command.CanExecute(); };
    Register(std::move(info));
}

bool ShortcutManager::Unregister(const std::string& id) {
    const auto it = std::find_if(items_.begin(), items_.end(), [&](const ShortcutInfo& s) { return s.Id == id; });
    if (it == items_.end()) return false;
    items_.erase(it);
    return true;
}

bool ShortcutManager::Rebind(const std::string& id, ImGuiKeyChord chord) {
    const auto it = std::find_if(items_.begin(), items_.end(), [&](const ShortcutInfo& s) { return s.Id == id; });
    if (it == items_.end() || chord == ImGuiKey_None) return false;
    const auto* other = FindByChord(chord);
    if (other && other != &*it) return false;
    it->Chord = chord;
    return true;
}

const ShortcutInfo* ShortcutManager::Find(const std::string& id) const {
    const auto it = std::find_if(items_.begin(), items_.end(), [&](const ShortcutInfo& s) { return s.Id == id; });
    return it == items_.end() ? nullptr : &*it;
}

const ShortcutInfo* ShortcutManager::FindByChord(ImGuiKeyChord chord) const {
    const auto it = std::find_if(items_.begin(), items_.end(), [&](const ShortcutInfo& s) { return s.Chord == chord; });
    return it == items_.end() ? nullptr : &*it;
}

std::string ShortcutManager::Text(const std::string& id) const {
    const auto* info = Find(id);
    return info ? std::string(ImGui::GetKeyChordName(info->Chord)) : std::string();
}

void ShortcutManager::Process() {
    if (!Enabled || ImGui::GetCurrentContext() == nullptr) return;
    // Snapshot ids: actions are allowed to modify the registry.
    std::vector<std::string> fired;
    const bool typing = ImGui::GetIO().WantTextInput;
    for (const auto& item : items_) {
        if (typing && !item.AllowWhileTyping && !HasCommandModifier(item.Chord)) continue;
        if (item.CanExecute && !item.CanExecute()) continue;
        const ImGuiInputFlags flags = ImGuiInputFlags_RouteGlobal | (item.Repeat ? ImGuiInputFlags_Repeat : 0);
        if (ImGui::Shortcut(item.Chord, flags)) fired.push_back(item.Id);
    }
    for (const auto& id : fired) {
        if (const auto* item = Find(id)) {
            const auto action = item->Action;
            action();
        }
    }
}

} // namespace cpptoolkit::ui
