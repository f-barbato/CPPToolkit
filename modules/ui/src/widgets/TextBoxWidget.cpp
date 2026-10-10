#include "cpptoolkit/ui/widgets/TextBoxWidget.h"

#include <utility>

#include "../../detail/TextInput.h"

namespace cpptoolkit::ui {
TextBoxWidget::TextBoxWidget(std::string label, mvvm::ObservableProperty<std::string>& bound,
                             TextChangedHandler onTextChanged)
    : OnTextChanged(std::move(onTextChanged)), label_(std::move(label)), bound_(bound) {}

void TextBoxWidget::Draw() {
    detail::DrawText(label_, bound_, buffer_, readOnly, detail::TextMode::SingleLine, OnTextChanged);
}

} // namespace cpptoolkit::ui
