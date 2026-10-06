#pragma once

#include <string>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Persistent container of child widgets, built once and drawn every frame.
// This is what turns ImGui's immediate-mode calls into a "retained" tree:
// the structure is created up front (Add/Remove), not rebuilt per frame.
class Panel : public Widget {

    public:

        Panel(const std::string& title) : _title(title) {}

        virtual void Draw() override {

            if(!this->Visible)
                return;

            ImGui::Begin(_title.c_str(), &this->_open, _flags);

            DrawChildren();

            ImGui::End();
        }

        void SetFlags(ImGuiWindowFlags flags) {
            _flags |= flags;
        }

        void ClearFlags(ImGuiWindowFlags flags) {
            _flags &= ~flags;
        }

        void SetOpen(bool open) {
            _open = open;
        }

        bool IsOpen() const {
            return _open;
        }

        void SetTitle(const std::string& title) {
            _title = title;
        }

        const std::string& GetTitle() const {
            return _title;
        }
        ImGuiWindowFlags GetFlags() const {
            return _flags;
        }

        bool HasFlag(ImGuiWindowFlags flag) const {
            return (_flags & flag) != 0;
        }

    private:
        ImGuiWindowFlags _flags = 0;
        bool _open = true;
        std::string _title;

};

} // namespace cpptoolkit::ui
