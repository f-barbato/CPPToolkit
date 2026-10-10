#pragma once

/** @file
 *  @brief Application lifecycle, runtime configuration and thread dispatch.
 */

#include "cpptoolkit/ui/Widget.h"
#include "cpptoolkit/ui/widgets/widgets.h"
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <type_traits>
#include <implot.h>

namespace cpptoolkit::ui {

/** @brief Own a root widget and run raylib/ImGui/ImPlot on the render thread.
 *  @note Configuration and factory access are render-thread-only.
 */
class CPPTOOLKIT_UI_EXPORT Application : public std::enable_shared_from_this<Application> {
public:
    /** @brief Synchronous handler for a standard exception escaping Run(). */
    using ErrorHandler = std::function<void(const std::exception&)>;
    /** @brief Retain the root widget.
     *  @param rootView Shared root, or nullptr.
     */
    explicit Application(std::shared_ptr<Widget> rootView = nullptr);
    /** @brief Release members; backend teardown is performed by Run(). */
    virtual ~Application() = default;
    /** @brief Initialize backends, build, render and tear down the tree.
     *  @return Zero on normal completion, one for a handled standard exception.
     */
    int Run();
    /** @brief Request render-loop termination on the render thread. */
    void Stop();
    /** @brief Add window flags, applying them at runtime if running.
     *  @param configFlags Flags to enable.
     *  @return This application.
     */
    Application& SetConfigFlags(unsigned int configFlags);
    /** @brief Remove window flags.
     *  @param configFlags Flags to disable.
     *  @return This application.
     */
    Application& ClearConfigFlags(unsigned int configFlags);
    /** @brief Set the window title, including at runtime.
     *  @param title Window title.
     *  @return This application.
     */
    Application& SetTitle(const std::string& title);
    /** @brief Set initial size or resize the running window.
     *  @param width Width in pixels.
     *  @param height Height in pixels.
     *  @return This application.
     */
    Application& SetInitialSize(int width, int height);
    /** @brief Set minimum window dimensions.
     *  @param width Minimum width.
     *  @param height Minimum height.
     *  @return This application.
     */
    Application& SetMinSize(int width, int height);
    /** @brief Set a borrowed window icon.
     *  @param icon Image whose pixel data must remain alive until Run() applies it.
     *  @return This application.
     */
    Application& SetIcon(const Image& icon);
    /** @brief Configure frame rate.
     *  @param targetFPS Desired frames per second.
     *  @return This application.
     */
    Application& SetTargetFPS(int targetFPS);
    /** @brief Set the initial or runtime ImGui theme.
     *  @param darkTheme True for dark, false for light.
     *  @return This application.
     */
    Application& SetDarkTheme(bool darkTheme);
    /** @brief Set the frame clear color.
     *  @param backgroundColor raylib color.
     *  @return This application.
     */
    Application& SetBackgroundColor(Color backgroundColor);
    /** @brief Replace default stderr error reporting.
     *  @param handler Optional synchronous exception handler; nullptr restores stderr.
     *  @return This application.
     */
    Application& SetOnError(ErrorHandler handler);
    /** @brief Query configured flags.
     *  @return Current flag mask.
     */
    [[nodiscard]] unsigned int GetConfigFlags() const { return _configFlags; }
    /** @brief Query the title.
     *  @return Reference to stored title.
     */
    [[nodiscard]] const std::string& GetTitle() const { return _title; }
    /** @brief Query configured width.
     *  @return Initial/requested width.
     */
    [[nodiscard]] int GetInitialWidth() const { return _initialWidth; }
    /** @brief Query configured height.
     *  @return Initial/requested height.
     */
    [[nodiscard]] int GetInitialHeight() const { return _initialHeight; }
    /** @brief Query target frame rate.
     *  @return Desired frames per second.
     */
    [[nodiscard]] int GetTargetFPS() const { return _targetFPS; }
    /** @brief Query the selected theme.
     *  @return True for dark.
     */
    [[nodiscard]] bool IsDarkTheme() const { return _darkTheme; }
    /** @brief Query the clear color.
     *  @return Current background color.
     */
    [[nodiscard]] Color GetBackgroundColor() const { return _backgroundColor; }
    /** @brief Access root ownership.
     *  @return Shared root reference, possibly null.
     */
    [[nodiscard]] const std::shared_ptr<Widget>& GetRootView() const { return _rootView; }

    /** @brief Create or retrieve the process-wide application for a root type.
     *  @tparam T Default-constructible Widget-derived root.
     *  @return Singleton application.
     *  @throws std::logic_error If already created with a different root type.
     */
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<Widget, T>>>
    static Application& Create() {
        static_assert(std::is_default_constructible_v<T>, "Root must be default-constructible");
        const auto type = std::type_index(typeid(T));
        if (!s_instance) {
            s_instance = std::make_shared<Application>(std::make_shared<T>());
            s_instanceType = type;
        } else if (s_instanceType != type) {
            throw std::logic_error("Application already created with a different root type");
        }
        return *s_instance;
    }
    /** @brief Compatibility alias for Create().
     *  @tparam T Default-constructible Widget-derived root.
     *  @return Singleton application.
     *  @throws std::logic_error If the application uses a different root type.
     */
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<Widget, T>>>
    static Application& GetInstance() { return Create<T>(); }
    /** @brief Access the singleton created by Create().
     *  @return Singleton application.
     *  @throws std::logic_error If Create() has not been called.
     */
    static Application& Instance();
    /** @brief Queue work on the single NotificationWorker thread.
     *  @param work Background task; own captured lifetimes and do not call ImGui directly.
     *  @note Tasks serialize; uncaught task exceptions follow NotificationWorker semantics.
     */
    static void RunAsync(std::function<void()> work);
    /** @brief Queue an action for the render thread.
     *  @param action Action drained at the start of a running render frame.
     */
    static void Dispatch(std::function<void()> action);

protected:
    /** @brief Check whether the loop should continue.
     *  @return Running flag and absence of a window-close request.
     */
    bool IsRunning();
    /** @brief Clear, invoke root Render() with raylib, then root Draw() with ImGui.
     *  @note Render precedes Dispatcher processing, which remains inside the ImGui
     *        frame. Dispatched changes therefore reach Render on the next frame.
     *        Both backend frame scopes are closed even when a widget throws.
     */
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
