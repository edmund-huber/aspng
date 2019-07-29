#include <algorithm>

#include "device.h"

bool Coord2d::operator==(Coord2d other) {
    return (this->x == other.x) && (this->y == other.y);
}

Patch *Device::flood(Png *png, size_t x, size_t y, Rgb color) {
    Patch *patch = new Patch();
    Patch visited;
    Device::_flood(png, x, y, color, patch, &visited);
    return patch;
}

void Device::_flood(Png *png, size_t x, size_t y, Rgb color, Patch *patch, Patch *visited) {
    if ((x < 0) || (x >= png->get_width())) {
        return;
    }
    if ((y < 0) || (y >= png->get_height())) {
        return;
    }
    if (std::find(visited->begin(), visited->end(), Coord2d(x, y)) != visited->end()) {
        return;
    }
    visited->push_back(Coord2d(x, y));
    if (png->get_pixel(x, y) == color) {
        patch->push_back(Coord2d(x, y));
        Device::_flood(png, x - 1, y, color, patch, visited);
        Device::_flood(png, x + 1, y, color, patch, visited);
        Device::_flood(png, x, y - 1, color, patch, visited);
        Device::_flood(png, x, y + 1, color, patch, visited);
    }
}
