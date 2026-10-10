#include "cpptoolkit/ui/widgets/TextWidget.h"

#include <utility>

namespace cpptoolkit::ui{
    TextWidget::TextWidget(std::string text) : text_(std::move(text)) {}
    TextWidget::TextWidget(mvvm::ObservableProperty<std::string>& bound) : bound_(&bound) {}

    void TextWidget::Draw(){
        ImGui::TextUnformatted(bound_ ? bound_->Get().c_str() : text_.c_str());
    }
} // namespace cpptoolkit::ui