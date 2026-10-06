#include "cpptoolkit/ui/widgets/TextWidget.h"

namespace cpptoolkit::ui{
    TextWidget::TextWidget(mvvm::ObservableProperty<std::string>& bound) : bound_(bound) {}

    void TextWidget::Draw(){ ImGui::TextUnformatted(bound_.Get().c_str()); }
} // namespace cpptoolkit::ui