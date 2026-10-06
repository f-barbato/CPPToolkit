#pragma once

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/ui/widgets/widgets.h"

#include "cpptoolkit/mvvm/Dispatcher.h"
#include "cpptoolkit/mvvm/NotificationWorker.h"

#include <raylib.h>
#include <rlImGui.h>

#include <imgui.h>
#include <implot.h>

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <type_traits>

namespace cpptoolkit::ui {
    
    // Owns the raylib window + ImGui/ImPlot contexts and drives the main
    // loop over a type-erased Widget tree (see Widget.h for the ambient
    // ViewModel/Application cascading that lets descendants reach back into
    // both without being threaded through constructors).
    //
    // Usage: build the singleton once via Create<T>() (T being the root
    // Widget, e.g. a View<T>), configure it with the fluent Set*() methods,
    // then call Run(). Any later code (including code running inside the
    // widget tree) can reach the same instance via Instance().
    class Application : public std::enable_shared_from_this<Application> {

        public:
            using ErrorHandler = std::function<void(const std::exception&)>;

            // Takes any Widget-derived root (e.g. a View<T>, which derives
            // from Widget) type-erased to shared_ptr<Widget>, so Application
            // itself doesn't need to be a class template: the shared_ptr
            // upcast happens implicitly at the call site.
            explicit Application(std::shared_ptr<Widget> rootView = nullptr);
            virtual ~Application() = default;

            int Run();
            void Stop();

            Application& SetConfigFlags(unsigned int configFlags);
            Application& ClearConfigFlags(unsigned int configFlags);
            Application& SetTitle(const std::string& title);
            Application& SetInitialSize(int width, int height);
            Application& SetMinSize(int width, int height);
            Application& SetIcon(const Image& icon);
            Application& SetTargetFPS(int targetFPS);
            Application& SetDarkTheme(bool darkTheme);
            Application& SetBackgroundColor(Color backgroundColor);
            // Invoked from Run() if an exception escapes the main loop,
            // instead of the default "log to stderr and return 1" handling.
            Application& SetOnError(ErrorHandler handler);

            [[nodiscard]] unsigned int GetConfigFlags() const { return _configFlags; }
            [[nodiscard]] const std::string& GetTitle() const { return _title; }
            [[nodiscard]] int GetInitialWidth() const { return _initialWidth; }
            [[nodiscard]] int GetInitialHeight() const { return _initialHeight; }
            [[nodiscard]] int GetTargetFPS() const { return _targetFPS; }
            [[nodiscard]] bool IsDarkTheme() const { return _darkTheme; }
            [[nodiscard]] Color GetBackgroundColor() const { return _backgroundColor; }
            [[nodiscard]] const std::shared_ptr<Widget>& GetRootView() const { return _rootView; }

            // Creates the process-wide Application instance, rooted at a
            // default-constructed widget of type T (e.g. a View<T>). Safe to
            // call multiple times with the *same* T (returns the existing
            // instance); calling it again with a *different* T is a usage
            // error and throws std::logic_error, since only one Application/
            // window is supported per process.
            template<typename T, typename = std::enable_if_t<std::is_base_of_v<Widget, T>>>
            static Application& Create(){
                static_assert(std::is_default_constructible_v<T>,
                    "Application::Create<T>: T must be default-constructible");

                const auto type = std::type_index(typeid(T));
                if (!s_instance) {
                    s_instance = std::make_shared<Application>(std::make_shared<T>());
                    s_instanceType = type;
                } else if (s_instanceType != type) {
                    throw std::logic_error(
                        "cpptoolkit::ui::Application::Create<T>() called again with a different "
                        "root widget type; only one Application is supported per process");
                }
                return *s_instance;
            }

            // Returns the instance created by Create<T>(). Throws
            // std::logic_error if Create<T>() hasn't been called yet.
            static Application& Instance();

            // Runs `work` in parallel on mvvm::NotificationWorker's single
            // background thread (never the render thread). Use this for
            // "event" producers (timers, simulated/sensor data, I/O
            // completions, ...): have `work` call Set()/Notify() on
            // ObservableProperty/Observable, which are mutex-protected, and
            // the next Draw() picks up the new value by polling — no
            // further synchronization needed. Never call ImGui APIs
            // directly from here; use Dispatch() for that instead.
            static void RunAsync(std::function<void()> work);

            // Queues `action` to run on the render thread, drained once per
            // frame at the top of the Run() loop. Only needed when a
            // background-thread handler (e.g. inside RunAsync()) must call
            // an ImGui API directly (e.g. ImGui::OpenPopup) — ImGui is not
            // thread-safe, so such calls must be marshaled here rather than
            // executed in place.
            static void Dispatch(std::function<void()> action);

        protected:
            bool IsRunning();
            virtual void Draw();

        private:
            static std::shared_ptr<Application> s_instance;
            static std::optional<std::type_index> s_instanceType;

            bool _running = false;

            unsigned int _configFlags = 0;
            std::string _title = "CPPToolkit App";
            int _initialWidth = 800;
            int _initialHeight = 600;
            int _minWidth = 0;
            int _minHeight = 0;
            bool _hasIcon = false;
            Image _icon{};
            int _targetFPS = 60;
            bool _darkTheme = false;
            Color _backgroundColor = RAYWHITE;
            ErrorHandler _onError;

            std::shared_ptr<Widget> _rootView;
    };

} // namespace cpptoolkit::ui