#include "ssd1309spi.hpp"

namespace ssd1309spi {

Rect clip_rect(Rect rect, int w, int h) {
    int r = rect.r(), b = rect.b();
    rect.x = clip(0, w, rect.x);
    rect.y = clip(0, h, rect.y);
    rect.w = clip(0, w, r) - rect.x;
    rect.h = clip(0, h, b) - rect.y;
    return rect;
}

}
