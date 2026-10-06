#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

// Persistent container of child widgets, built once and drawn every frame.
// This is what turns ImGui's immediate-mode calls into a "retained" tree:
// the structure is created up front (Add/Remove), not rebuilt per frame.
class Panel : public Widget {
};

} // namespace cpptoolkit::ui
