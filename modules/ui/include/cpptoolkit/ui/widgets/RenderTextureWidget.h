#pragma once

/** @file
 *  @brief Owned offscreen raylib rendering presented as a retained ImGui image.
 */

#include <functional>
#include <string>

#include <cpptoolkit/ui/Widget.h>

namespace cpptoolkit::ui {

/** @brief Own a fixed-resolution framebuffer and display it with vertically corrected UVs.
 *
 * Build(), Render(), Draw(), Destroy() and destruction run on the render thread.
 * Build requires a ready raylib window; Destroy/destruction must precede window
 * teardown. The callback can use GetViewModel<T>() to share ambient MVVM state.
 * Resources stay in the widget, not the ViewModel. No simulation timing is implied.
 * Child Render() calls occur after leaving this framebuffer; children are not
 * automatically rendered into it. Draw() displays the image, then visible children.
 */
class CPPTOOLKIT_UI_EXPORT RenderTextureWidget : public Widget {
public:
    /** @brief Called with this widget while its offscreen target is active. */
    using RenderHandler = std::function<void(RenderTextureWidget&)>;
    /** @brief Configure a fixed-size offscreen target.
     *  @param label Display label (also scoped with this widget's identity).
     *  @param width Positive framebuffer width in pixels.
     *  @param height Positive framebuffer height in pixels.
     *  @param onRender Optional raylib callback, after clearing the target.
     *  @throws std::invalid_argument If either dimension is nonpositive.
     */
    RenderTextureWidget(std::string label, int width, int height, RenderHandler onRender = {});
    /** @brief Release the framebuffer if Destroy() has not already done so. */
    ~RenderTextureWidget() override;
    /** @brief Framebuffer ownership cannot be copied. */
    RenderTextureWidget(const RenderTextureWidget&) = delete;
    /** @brief Framebuffer ownership cannot be copied.
     *  @param other Unused source.
     *  @return Not available; assignment is deleted.
     */
    RenderTextureWidget& operator=(const RenderTextureWidget& other) = delete;
    /** @brief Allocate the framebuffer, then build the widget tree.
     *  @throws std::logic_error If already built or the raylib window is not ready.
     *  @throws std::runtime_error If framebuffer allocation fails.
     */
    void Build() override;
    /** @brief Clear/render offscreen, restore the screen target, then render children.
     *  @note Callback must balance its camera/shader scopes, must not change the
     *        render target or begin/end the application frame, and must not call ImGui.
     *        EndTextureMode() is guaranteed even if the callback throws.
     *  @throws std::logic_error If called before Build(), unless RenderEnabled is false.
     */
    void Render() override;
    /** @brief Show the rendered image at the configured size when Visible.
     *  @throws std::logic_error If called before Build(), unless Visible is false.
     */
    void Draw() override;
    /** @brief Tear down children and release the framebuffer; safe to call repeatedly. */
    void Destroy() override;
    /** @brief Access the borrowed framebuffer; never unload it externally.
     *  @return Owned target, valid from successful Build() until Destroy().
     *  @throws std::logic_error If no framebuffer is allocated.
     */
    const RenderTexture2D& GetRenderTexture() const;
    /** @brief Set image display dimensions without changing framebuffer resolution.
     *  @param size Positive finite display dimensions.
     *  @throws std::invalid_argument If either dimension is nonpositive or nonfinite.
     */
    void SetDisplaySize(ImVec2 size);
    /** @brief Query configured display dimensions.
     *  @return Image size in pixels.
     */
    ImVec2 GetDisplaySize() const { return size_; }
    /** @brief Offscreen raylib callback; assignments are render-thread-only. */
    RenderHandler OnRenderTexture;
    /** @brief Color used to clear the offscreen target every enabled render phase. */
    Color BackgroundColor = BLACK;

private:
    void Release() noexcept;
    std::string label_;
    int width_;
    int height_;
    ImVec2 size_;
    RenderTexture2D texture_{};
};

} // namespace cpptoolkit::ui
