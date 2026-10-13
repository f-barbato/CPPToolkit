#pragma once

#include "TestUIState.h"

/** @brief Adds every Input-tab control (selection, numeric, text, file, date/time, color) to @p panel. */
void BuildInputExamples(cpptoolkit::ui::Panel& panel, TestUIState& vm);

/**
 * @brief Builds the widget gallery (menu bar, tab bar and root-level modal) under @p root.
 *
 * Tabs: Input, Feedback, Data, Navigation, Layout, Style, Overlay.
 */
void BuildTestUIWidgets(cpptoolkit::ui::Widget& root, TestUIState& vm);
