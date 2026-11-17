#pragma once

namespace jv1010 {
namespace ui {

// Draws the Roland JV-1010 front-panel silkscreen as pure anti-aliased vector
// geometry directly over the current background (no box/bezel/photo artifacts).
void drawJV1010Silkscreen(float targetHeight = 52.0f);

} // namespace ui
} // namespace jv1010
