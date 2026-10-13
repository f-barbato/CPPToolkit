#pragma once

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>

#include <cpptoolkit/mvvm/ObservableProperty.h>
#include <cpptoolkit/mvvm/UndoStack.h>

namespace cpptoolkit::ui::detail {

inline int ResizeTextBuffer(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto& buffer = *static_cast<std::vector<char>*>(data->UserData);
        buffer.resize(static_cast<std::size_t>(data->BufSize));
        data->Buf = buffer.data();
    }
    return 0;
}

enum class TextMode { SingleLine, Multiline, Password, Search };

inline void DrawText(const std::string& label, mvvm::ObservableProperty<std::string>& bound,
                     std::vector<char>& buffer, bool readOnly, TextMode mode,
                     const std::function<void(const std::string&)>& handler,
                     mvvm::UndoStack* history = nullptr) {
    const auto previous = bound.Get();
    buffer.assign(previous.begin(), previous.end());
    buffer.push_back('\0');
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
    if (readOnly) flags |= ImGuiInputTextFlags_ReadOnly;
    bool changed = false;
    switch (mode) {
    case TextMode::SingleLine:
        changed = ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                                  flags, ResizeTextBuffer, &buffer);
        break;
    case TextMode::Multiline:
        changed = ImGui::InputTextMultiline(label.c_str(), buffer.data(), buffer.size(),
                                            ImVec2(0, 0), flags, ResizeTextBuffer, &buffer);
        break;
    case TextMode::Password:
        changed = ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                                  flags | ImGuiInputTextFlags_Password, ResizeTextBuffer, &buffer);
        break;
    case TextMode::Search:
        changed = ImGui::InputTextWithHint(label.c_str(), "Search...", buffer.data(), buffer.size(),
                                          flags, ResizeTextBuffer, &buffer);
        break;
    }
    const bool released = history && ImGui::IsItemDeactivated();
    if (changed) {
        const std::string edited(buffer.data());
        if (edited != previous) {
            bound.Set(edited);
            if (history) mvvm::RecordChange(*history, bound, previous, edited, label, true);
            if (handler) handler(edited);
        }
    }
    if (released) history->BreakMerge();
}

} // namespace cpptoolkit::ui::detail
