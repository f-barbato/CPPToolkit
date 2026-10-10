#include <cpptoolkit/ui/widgets/RenderTextureWidget.h>

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cpptoolkit::ui {

RenderTextureWidget::RenderTextureWidget(std::string label, int width, int height,
                                         RenderHandler onRender)
    : OnRenderTexture(std::move(onRender)), label_(std::move(label)), width_(width),
      height_(height), size_(static_cast<float>(width), static_cast<float>(height)) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("RenderTextureWidget requires positive framebuffer dimensions");
}

RenderTextureWidget::~RenderTextureWidget() { Release(); }

void RenderTextureWidget::Release() noexcept {
    if (texture_.id) {
        UnloadRenderTexture(texture_);
        texture_ = {};
    }
}

void RenderTextureWidget::Build() {
    if (texture_.id) throw std::logic_error("RenderTextureWidget is already built");
    if (!IsWindowReady()) throw std::logic_error("RenderTextureWidget requires a ready raylib window");
    texture_ = LoadRenderTexture(width_, height_);
    if (!IsRenderTextureValid(texture_)) {
        Release();
        throw std::runtime_error("RenderTextureWidget framebuffer allocation failed");
    }
    try {
        Widget::Build();
    } catch (...) {
        Release();
        throw;
    }
}

const RenderTexture2D& RenderTextureWidget::GetRenderTexture() const {
    if (!texture_.id) throw std::logic_error("RenderTextureWidget is not built");
    return texture_;
}

void RenderTextureWidget::SetDisplaySize(ImVec2 size) {
    if (!std::isfinite(size.x) || !std::isfinite(size.y) || size.x <= 0 || size.y <= 0)
        throw std::invalid_argument("RenderTextureWidget requires positive finite display dimensions");
    size_ = size;
}

void RenderTextureWidget::Render() {
    if (!RenderEnabled) return;
    BeginTextureMode(GetRenderTexture());
    {
        struct TargetGuard {
            ~TargetGuard() { EndTextureMode(); }
        } targetGuard;
        ClearBackground(BackgroundColor);
        OnRender();
        if (OnRenderTexture) OnRenderTexture(*this);
    }
    RenderChildren();
}

void RenderTextureWidget::Draw() {
    if (!Visible) return;
    const auto& target = GetRenderTexture();
    ImGui::PushID(this);
    ImGui::TextUnformatted(label_.c_str());
    ImGui::Image(static_cast<ImTextureID>(target.texture.id), size_, ImVec2(0, 1), ImVec2(1, 0));
    DrawChildren();
    ImGui::PopID();
}

void RenderTextureWidget::Destroy() {
    struct ReleaseGuard {
        RenderTextureWidget& widget;
        ~ReleaseGuard() { widget.Release(); }
    } releaseGuard{*this};
    Widget::Destroy();
}

} // namespace cpptoolkit::ui
