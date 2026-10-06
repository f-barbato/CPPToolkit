#include "cpptoolkit/ui/Application.h"

#include <print>

namespace cpptoolkit::ui {

    Application::Application(std::shared_ptr<Widget> rootView) : _rootView(std::move(rootView)) {}
    
    int Application::Run() {
        
        try{

            this->_running = true;

            if(this->_configFlags){
                ::SetConfigFlags(this->_configFlags);
            }

            InitWindow(this->_initialWidth, this->_initialHeight, this->_title.c_str());
            ::SetTargetFPS(this->_targetFPS);
            rlImGuiSetup(this->_darkTheme);
            ImPlot::CreateContext();

            if(this->_rootView){
                this->_rootView->Build();
            }

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
        this->_running = false;
    }

    bool Application::IsRunning() {
        return this->_running && !WindowShouldClose();
    }

    void Application::Draw() {
        BeginDrawing();
        ClearBackground(this->_backgroundColor);

        rlImGuiBegin();

        if (this->_rootView) {
            this->_rootView->Draw();
        }

        rlImGuiEnd();

        EndDrawing();
    }

    Application& Application::SetConfigFlags(unsigned int configFlags) {
        this->_configFlags = configFlags;
        std::println("Config flags set to: {}", this->_configFlags);
        return *this;
    }

    Application& Application::SetTitle(const std::string& title) {
        this->_title = title;
        std::println("Title set to: {}", this->_title);
        return *this;
    }

    Application& Application::SetInitialSize(int width, int height) {
        this->_initialWidth = width;
        this->_initialHeight = height;
        std::println("Initial size set to: {}x{}", this->_initialWidth, this->_initialHeight);
        return *this;
    }

    Application& Application::SetTargetFPS(int targetFPS) {
        this->_targetFPS = targetFPS;
        std::println("Target FPS set to: {}", this->_targetFPS);
        return *this;
    }

    Application& Application::SetDarkTheme(bool darkTheme) {
        this->_darkTheme = darkTheme;
        std::println("Dark theme set to: {}", this->_darkTheme);
        return *this;
    }

    Application& Application::SetBackgroundColor(Color backgroundColor) {
        this->_backgroundColor = backgroundColor;
        std::println("Background color set.");
        return *this;
    }


} // namespace cpptoolkit::ui