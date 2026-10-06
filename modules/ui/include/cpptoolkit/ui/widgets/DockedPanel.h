#pragma once

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui{

    class DockedPanel : public Widget{

    public:

        void PreBuild() override {
            #ifdef IMGUI_HAS_DOCK
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            #endif
        }

        void Draw() override {
            if(!this->Visible)
                return;

            #ifdef IMGUI_HAS_DOCK
            ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);
            #endif

            DrawChildren();
        }
    };
    
}