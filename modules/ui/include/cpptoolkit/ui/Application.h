#pragma once

#include "cpptoolkit/ui/widgets/widgets.h"

#include <raylib.h>
#include <rlImGui.h>

#include <imgui.h>
#include <implot.h>

namespace cpptoolkit::ui {

    class Application{

        public:
            Application() = default;
            ~Application() = default;

            int Run();
            void Stop();

        protected:
            bool IsRunning();
            virtual void Draw();

        private:
            bool running = false;

            std::string title = "CPPToolkit App";
            int initialWidth = 800;
            int initialHeight = 600;
            int targetFPS = 60;
            bool darkTheme = false;
            Color backgroundColor = RAYWHITE;
    };

} // namespace cpptoolkit::ui