#include "cpptoolkit/ui/Application.h"


namespace cpptoolkit::ui {
    int Application::Run() {
        
        try{

            this->running = true;

            InitWindow(this->initialWidth, this->initialHeight, this->title.c_str());
            SetTargetFPS(this->targetFPS);
            rlImGuiSetup(this->darkTheme);
            ImPlot::CreateContext();

            while (this->IsRunning()) {
                this->Draw();
            }

            ImPlot::DestroyContext();

            rlImGuiShutdown();
            CloseWindow();

        } catch (const std::exception& e) {
            return 1;
        }

        return 0;
    }

    void Application::Stop() {
        this->running = false;
    }

    bool Application::IsRunning() {
        return this->running && !WindowShouldClose();
    }

    void Application::Draw() {
        BeginDrawing();
        ClearBackground(this->backgroundColor);

        rlImGuiBegin();
        ImGui::ShowDemoWindow();
        rlImGuiEnd();

        EndDrawing();
    }

} // namespace cpptoolkit::ui