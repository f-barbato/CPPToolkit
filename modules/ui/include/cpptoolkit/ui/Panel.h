#pragma once

/** @file
 *  @brief Basic retained widget container.
 */

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "cpptoolkit/ui/Widget.h"

namespace cpptoolkit::ui {

/** @brief Container using Widget's child ownership, lifecycle and default vertical flow. */
class Panel : public Widget {
};

} // namespace cpptoolkit::ui
