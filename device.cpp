#include <algorithm>

#include "device.h"

bool Coord2d::operator==(Coord2d other) {
    return (this->x == other.x) && (this->y == other.y);
}

Patch *Device::flood(Png *png, size_t x, size_t y, Rgb color) {
    Patch *patch = new Patch();
    Patch visited;
    Device::flood_helper(png, x, y, color, patch, &visited);
    return patch;
}

void Device::flood_helper(Png *png, size_t x, size_t y, Rgb color, Patch *patch, Patch *visited) {
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
        Device::flood_helper(png, x - 1, y, color, patch, visited);
        Device::flood_helper(png, x + 1, y, color, patch, visited);
        Device::flood_helper(png, x, y - 1, color, patch, visited);
        Device::flood_helper(png, x, y + 1, color, patch, visited);
    }
}

void Device::link_find_neighbors(Device ***assigned, size_t w, size_t h, std::set<Device *> *neighbors) {
    Patch *patch = this->all_patches();
    for (auto it = patch->begin(); it != patch->end(); it++) {
        Coord2d xy = *it;
        this->maybe_neighbor(assigned, w, h, xy.x - 1, xy.y, neighbors);
        this->maybe_neighbor(assigned, w, h, xy.x + 1, xy.y, neighbors);
        this->maybe_neighbor(assigned, w, h, xy.x, xy.y - 1, neighbors);
        this->maybe_neighbor(assigned, w, h, xy.x, xy.y + 1, neighbors);
    }
}

void Device::maybe_neighbor(Device ***assigned, size_t w, size_t h, size_t x, size_t y, std::set<Device *> *neighbors) {
    if ((x < 0) || (x >= w)) {
        return;
    }
    if ((y < 0) || (y >= h)) {
        return;
    }
    if (assigned[x][y] != this) {
        neighbors->insert(assigned[x][y]);
    }
}
