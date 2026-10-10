#pragma once

/** @file
 *  @brief Viewport dockspace container.
 */

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui{

    /** @brief Enable docking and render children inside a viewport dockspace. */
    class DockedPanel : public Widget{

    public:

        /** @brief Enable docking when the ImGui docking build is available. */
        void PreBuild() override {
            #ifdef IMGUI_HAS_DOCK
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            #endif
        }

        /** @brief Submit the dockspace and draw visible child windows. */
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