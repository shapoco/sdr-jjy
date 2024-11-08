#pragma once

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
};

Rect clip_rect(const Rect rect, int w, int h);

}
