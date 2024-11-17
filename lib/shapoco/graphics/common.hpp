#pragma once

#include "shapoco/math_utils.hpp"

namespace shapoco::graphics {

class Rect {
public:
    int x, y, w, h;
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) { }
    int r() const { return x + w; }
    int b() const { return y + h; }
    int cx() const { return (x + w / 2); }
    int cy() const { return (y + h / 2); }
    int contains(int dx, int dy) const {
        return
            x <= dx && dx < (x + w) && 
            y <= dy && dy < (y + h);
    }

    void offsetSelf(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void intersectSelf(Rect other) {
        int r = SHPC_MIN(x + w, other.x + other.w);
        int b = SHPC_MIN(y + h, other.y + other.h);
        x = SHPC_MAX(x, other.x);
        y = SHPC_MAX(y, other.y);
        w = r - x;
        h = b - y;
    }
    
    void inflateSelf(int xSize, int ySize) {
        x -= xSize;
        y -= ySize;
        w += xSize * 2;
        h += ySize * 2;
    }
};

Rect clip_rect(const Rect rect, int w, int h);

}
