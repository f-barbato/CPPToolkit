#pragma once

/** @file
 *  @brief raylib / rlImGui / ImPlot application lifecycle and configuration.
 */

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/ui/widgets/widgets.h"

#include <raylib.h>
#include <rlImGui.h>

#include <imgui.h>
#include <implot.h>

#include <memory>
#include <string>

namespace cpptoolkit::ui {
    
    /** @brief Own a root widget and run its rendering lifecycle.
     *
     * Configure before Run(). Rendering, Stop() and configuration changes are
     * render-thread operations, not a thread-safe application control API.
     */
    class Application : public std::enable_shared_from_this<Application> {

        public:
            /** @brief Retain a type-erased root widget.
             *  @param rootView Shared root, or nullptr for an empty application.
             */
            explicit Application(std::shared_ptr<Widget> rootView = nullptr);
            /** @brief Release application members; window teardown is performed by Run(). */
            ~Application() = default;

            /** @brief Initialize backends, build the root, render and tear down.
             *  @return 0 on normal completion, 1 if a standard exception is caught.
             */
            int Run();
            /** @brief Request loop termination from the application's thread. */
            void Stop();

            /** @brief Configure raylib window flags for the next Run().
             *  @param configFlags Bitmask of raylib window flags.
             *  @return This application for chaining.
             */
            Application& SetConfigFlags(unsigned int configFlags);
            /** @brief Set the next window title.
             *  @param title Window title.
             *  @return This application.
             */
            Application& SetTitle(const std::string& title);
            /** @brief Set the next initial window dimensions.
             *  @param width Width in pixels.
             *  @param height Height in pixels.
             *  @return This application.
             */
            Application& SetInitialSize(int width, int height);
            /** @brief Configure raylib's frame-rate target.
             *  @param targetFPS Desired frames per second.
             *  @return This application.
             */
            Application& SetTargetFPS(int targetFPS);
            /** @brief Select the initial ImGui theme.
             *  @param darkTheme True for dark, false for light.
             *  @return This application.
             */
            Application& SetDarkTheme(bool darkTheme);
            /** @brief Set the frame clear color.
             *  @param backgroundColor raylib color.
             *  @return This application.
             */
            Application& SetBackgroundColor(Color backgroundColor);

            /** @brief Retrieve one static application instance per root type.
             *  @tparam T Default-constructible Widget-derived root.
             *  @return Application retaining a shared instance of T.
             */
            template<typename T, typename = std::enable_if_t<std::is_base_of_v<Widget, T>>>
            static Application& GetInstance(){
                static Application app(std::make_shared<T>());
                return app;
            }

        protected:
            /** @brief Check the explicit running flag and window-close request.
             *  @return True while the render loop should continue.
             */
            bool IsRunning();
            /** @brief Render one frame, clearing the background and drawing the root. */
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