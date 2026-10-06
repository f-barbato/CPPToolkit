#include "cpptoolkit/ui/Application.h"

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

            if(this->_rootView){
                this->_rootView->Destroy();
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
        return *this;
    }

    Application& Application::SetTitle(const std::string& title) {
        this->_title = title;
        return *this;
    }

    Application& Application::SetInitialSize(int width, int height) {
        this->_initialWidth = width;
        this->_initialHeight = height;
        return *this;
    }

    Application& Application::SetTargetFPS(int targetFPS) {
        this->_targetFPS = targetFPS;
        return *this;
    }

    Application& Application::SetDarkTheme(bool darkTheme) {
        this->_darkTheme = darkTheme;
        return *this;
    }

    Application& Application::SetBackgroundColor(Color backgroundColor) {
        this->_backgroundColor = backgroundColor;
        return *this;
    }


} // namespace cpptoolkit::ui