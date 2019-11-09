#include <algorithm>

#include "device.h"

Patch Device::flood(Png *png, size_t x, size_t y, Rgb color) {
    Patch patch, visited;
    Device::flood_helper(png, x, y, color, patch, visited);
    return patch;
}

void Device::flood_helper(Png *png, size_t x, size_t y, Rgb color, Patch &patch, Patch &visited) {
    if ((x < 0) || (x >= png->get_width())) {
        return;
    }
    if ((y < 0) || (y >= png->get_height())) {
        return;
    }
    if (std::find(visited.begin(), visited.end(), Coord(x, y)) != visited.end()) {
        return;
    }
    visited.push_back(Coord(x, y));
    if (png->get_pixel(x, y) == color) {
        patch.push_back(Coord(x, y));
        Device::flood_helper(png, x - 1, y, color, patch, visited);
        Device::flood_helper(png, x + 1, y, color, patch, visited);
        Device::flood_helper(png, x, y - 1, color, patch, visited);
        Device::flood_helper(png, x, y + 1, color, patch, visited);
    }
}
