#include "cpptoolkit/ui/Application.h"

#include "cpptoolkit/mvvm/Dispatcher.h"
#include "cpptoolkit/mvvm/NotificationWorker.h"

#include <iostream>
#include <utility>

namespace cpptoolkit::ui {

    std::shared_ptr<Application> Application::s_instance;
    std::optional<std::type_index> Application::s_instanceType;

    Application::Application(std::shared_ptr<Widget> rootView) : _rootView(std::move(rootView)) {}

    int Application::Run() {

        this->_running = true;

        if (this->_configFlags) {
            ::SetConfigFlags(this->_configFlags);
        }

        InitWindow(this->_initialWidth, this->_initialHeight, this->_title.c_str());

        if (this->_minWidth > 0 && this->_minHeight > 0) {
            SetWindowMinSize(this->_minWidth, this->_minHeight);
        }
        if (this->_hasIcon) {
            SetWindowIcon(this->_icon);
        }

        ::SetTargetFPS(this->_targetFPS);
        rlImGuiSetup(this->_darkTheme);
        ImPlot::CreateContext();

        if (this->_rootView) {
            this->_rootView->SetApplication(this);
            this->_rootView->Build();
        }

        // Guarantees ImPlot/rlImGui/raylib teardown runs exactly once, no
        // matter how the loop below is exited: normally, via Stop(), or by
        // an exception propagating out of Draw()/a widget. Without this,
        // an exception used to skip straight to the catch clause below and
        // leak the ImGui/ImPlot contexts and the raylib window.
        struct ShutdownGuard {
            Application* self;
            ~ShutdownGuard() {
                if (self->_rootView) self->_rootView->Destroy();
                ImPlot::DestroyContext();
                rlImGuiShutdown();
                CloseWindow();
            }
        } shutdownGuard{this};

        try {
            while (this->IsRunning()) {
                // Drains actions posted from background threads (e.g. a
                // net transport callback) so they run here, on the single
                // render thread, before this frame's widgets are drawn.
                mvvm::Dispatcher::Main().ProcessPending();
                this->Draw();
            }
        } catch (const std::exception& e) {
            if (this->_onError) {
                this->_onError(e);
            } else {
                std::cerr << "cpptoolkit::ui::Application: unhandled exception: " << e.what() << std::endl;
            }
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
        this->_configFlags |= configFlags;
        // SetConfigFlags() only affects the *next* InitWindow() call; once
        // the window already exists, flags are toggled via SetWindowState()
        // instead (mirrors ClearConfigFlags()'s use of ClearWindowState()).
        if (this->_running) {
            SetWindowState(configFlags);
        }
        return *this;
    }

    Application& Application::ClearConfigFlags(unsigned int configFlags) {
        this->_configFlags &= ~configFlags;
        if (this->_running) {
            ClearWindowState(configFlags);
        }
        return *this;
    }

    Application& Application::SetTitle(const std::string& title) {
        this->_title = title;
        if (this->_running) {
            SetWindowTitle(this->_title.c_str());
        }
        return *this;
    }

    Application& Application::SetInitialSize(int width, int height) {
        this->_initialWidth = width;
        this->_initialHeight = height;
        if (this->_running) {
            SetWindowSize(width, height);
        }
        return *this;
    }

    Application& Application::SetMinSize(int width, int height) {
        this->_minWidth = width;
        this->_minHeight = height;
        if (this->_running) {
            SetWindowMinSize(width, height);
        }
        return *this;
    }

    Application& Application::SetIcon(const Image& icon) {
        this->_icon = icon;
        this->_hasIcon = true;
        if (this->_running) {
            SetWindowIcon(this->_icon);
        }
        return *this;
    }

    Application& Application::SetTargetFPS(int targetFPS) {
        this->_targetFPS = targetFPS;
        if (this->_running) {
            ::SetTargetFPS(targetFPS);
        }
        return *this;
    }

    Application& Application::SetDarkTheme(bool darkTheme) {
        this->_darkTheme = darkTheme;
        // rlImGuiSetup() only applies the initial theme at startup; once
        // the ImGui context exists, toggling at runtime just needs the
        // matching style preset re-applied directly.
        if (this->_running) {
            if (darkTheme) ImGui::StyleColorsDark();
            else ImGui::StyleColorsLight();
        }
        return *this;
    }

    Application& Application::SetBackgroundColor(Color backgroundColor) {
        this->_backgroundColor = backgroundColor;
        return *this;
    }

    Application& Application::SetOnError(ErrorHandler handler) {
        this->_onError = std::move(handler);
        return *this;
    }

    Application& Application::Instance() {
        if (!s_instance) {
            throw std::logic_error(
                "cpptoolkit::ui::Application::Instance() called before Create<T>()");
        }
        return *s_instance;
    }

    void Application::RunAsync(std::function<void()> work) {
        mvvm::NotificationWorker::Instance().Post(std::move(work));
    }

    void Application::Dispatch(std::function<void()> action) {
        mvvm::Dispatcher::Main().Post(std::move(action));
    }

} // namespace cpptoolkit::ui