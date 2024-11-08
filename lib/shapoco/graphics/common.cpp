#include "shapoco/math_utils.hpp"
#include "shapoco/graphics/common.hpp"

namespace shapoco::graphics {

Rect clip_rect(Rect rect, int w, int h) {
    int r = rect.r(), b = rect.b();
    rect.x = SHPC_CLIP(0, w, rect.x);
    rect.y = SHPC_CLIP(0, h, rect.y);
    rect.w = SHPC_CLIP(0, w, r) - rect.x;
    rect.h = SHPC_CLIP(0, h, b) - rect.y;
    return rect;
}

}
