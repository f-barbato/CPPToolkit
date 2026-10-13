#pragma once

#include <array>
#include <deque>
#include <string>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/ui.h>

#include "TestUIState.h"

namespace demo {

/** @brief Shared ViewModel of the demo: telemetry, text, font, palette and undo history. */
class DemoViewModel : public cpptoolkit::mvvm::ObservableObject {
public:
    using Color = std::array<float, 4>;

    /** @brief One editable palette color bound to a color picker. */
    struct PaletteEntry {
        PaletteEntry(const char* label, ImVec4 cpptoolkit::ui::Palette::*member, const Color& initial);
        const char* Label;
        ImVec4 cpptoolkit::ui::Palette::*Member;
        cpptoolkit::mvvm::ObservableProperty<Color> Value;
    };

    DemoViewModel();

    cpptoolkit::mvvm::ObservableProperty<float> SensorValue{this, "SensorValue", 0};
    cpptoolkit::mvvm::ObservableProperty<float> Amplitude{this, "Amplitude", 1};
    cpptoolkit::mvvm::ObservableProperty<std::string> Status{this, "Status", "Running"};
    cpptoolkit::mvvm::ObservableProperty<std::string> TextInput{this, "TextInput", ""};
    cpptoolkit::mvvm::ObservableProperty<std::string> FontPath{this, "FontPath", "C:\\Windows\\Fonts\\segoeui.ttf"};
    cpptoolkit::mvvm::ObservableProperty<float> FontSize{this, "FontSize", 16.0f};
    cpptoolkit::mvvm::ObservableProperty<bool> Closable{this, "Closable", true};

    /** @brief Editable palette colors; user edits are applied to the global style by the controls panel. */
    std::deque<PaletteEntry> PaletteColors;

    TestUIState Gallery;
    /** @brief History shared by the Amplitude sliders and the text input. */
    cpptoolkit::mvvm::UndoStack History;
    cpptoolkit::mvvm::Command ResetCommand{[this] {
        phase_ = 0;
        Status.Set("Reset");
    }};

    /** @brief Build a Palette from the current picker values. */
    cpptoolkit::ui::Palette CurrentPalette();
    /** @brief Copy a Palette into the picker values. */
    void LoadPalette(const cpptoolkit::ui::Palette& palette);
    /** @brief Advance the simulated sensor and the gallery. */
    void Tick(float dt);

private:
    float phase_ = 0;
};

} // namespace demo
