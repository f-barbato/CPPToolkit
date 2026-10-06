#pragma once

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/ui/widgets/widgets.h"

#include <raylib.h>
#include <rlImGui.h>

#include <imgui.h>
#include <implot.h>

#include <memory>
#include <string>

namespace cpptoolkit::ui {
    
    class Application : public std::enable_shared_from_this<Application> {

        public:
            // Takes any Widget-derived root (e.g. a View<T>, which derives
            // from Widget) type-erased to shared_ptr<Widget>, so Application
            // itself doesn't need to be a class template: the shared_ptr
            // upcast happens implicitly at the call site.
            explicit Application(std::shared_ptr<Widget> rootView = nullptr);
            ~Application() = default;

            int Run();
            void Stop();

            Application& SetConfigFlags(unsigned int configFlags);
            Application& SetTitle(const std::string& title);
            Application& SetInitialSize(int width, int height);
            Application& SetTargetFPS(int targetFPS);
            Application& SetDarkTheme(bool darkTheme);
            Application& SetBackgroundColor(Color backgroundColor);

            template<typename T, typename = std::enable_if_t<std::is_base_of_v<Widget, T>>>
            static Application& GetInstance(){
                static Application app(std::make_shared<T>());
                return app;
            }

        protected:
            bool IsRunning();
            virtual void Draw();

        private:
            bool _running = false;

            unsigned int _configFlags = 0;
            std::string _title = "CPPToolkit App";
            int _initialWidth = 800;
            int _initialHeight = 600;
            int _targetFPS = 60;
            bool _darkTheme = false;
            Color _backgroundColor = RAYWHITE;

            std::shared_ptr<Widget> _rootView;
    };

} // namespace cpptoolkit::ui