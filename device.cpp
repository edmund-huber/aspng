#include "common.h"
#include "device.h"

void Device::add_port(std::shared_ptr<Port> p) {
    this->ports.push_back(p);
}

std::list<std::shared_ptr<Port>> Device::all_ports(void) {
    return this->ports;
}

Patch Device::flood(AspngSurface *surface, Coord start_coord, Rgb color) {
    Patch patch, visited;
    std::list<Coord> to_visit;
    to_visit.push_back(start_coord);
    while (!to_visit.empty()) {
        Coord current = to_visit.back();
        to_visit.pop_back();

        if (surface->get_pixel(current.x, current.y) == color) {
            patch.insert(current);
            visited.insert(current);

            Device::flood_helper(Coord(current.x - 1, current.y), surface, visited, to_visit);
            Device::flood_helper(Coord(current.x + 1, current.y), surface, visited, to_visit);
            Device::flood_helper(Coord(current.x, current.y - 1), surface, visited, to_visit);
            Device::flood_helper(Coord(current.x, current.y + 1), surface, visited, to_visit);
        }
    }

    return patch;
}

void Device::flood_helper(Coord coord, AspngSurface *surface, Patch &visited, std::list<Coord> &to_visit) {
    if ((coord.x < 0) || (coord.x >= surface->get_width())) {
        return;
    }
    if ((coord.y < 0) || (coord.y >= surface->get_height())) {
        return;
    }
    if (visited.find(coord) == visited.end()) {
        to_visit.push_back(coord);
    }
}

// This doesn't return Patch, because Patches must be contiguous, but the union
// of all Patches in a device might not be.
std::set<Coord> Device::all_patches_combined(void) {
    auto all_patches = this->all_patches();
    std::set<Coord> all_patches_combined;
    for (auto i = all_patches.begin(); i != all_patches.end(); i++) {
        auto patch = *i;
        for (auto j = patch->begin(); j != patch->end(); j++) {
            auto coord = *j;
            all_patches_combined.insert(coord);
        }
    }
    return all_patches_combined;
}

BoundingBox Device::get_bounding_box(void) {
    BoundingBox bounding_box;
    auto all_patches = this->all_patches();
    for (auto i = all_patches.begin(); i != all_patches.end(); i++) {
        auto patch = *i;
        for (auto j = patch->begin(); j != patch->end(); j++) {
            auto coord = *j;
            bounding_box.include(coord);
        }
    }
    return bounding_box;
}

Patch *Device::find_patch_containing(Coord coord) {
    auto all_patches = this->all_patches();
    for (auto i = all_patches.begin(); i != all_patches.end(); i++) {
        auto patch = *i;
        if (patch->find(coord) != patch->end()) {
            return patch;
        }
    }
    ASSERT(0);
}

void Device::draw_helper(AspngSurface *surface, std::list<Patch *> patches) {
    for (auto i = patches.begin(); i != patches.end(); i++) {
        auto patch = *i;
        for (auto j = patch->begin(); j != patch->end(); j++) {
            auto coord = *j;
            ASSERT((coord.x >= 0) && (coord.x < surface->get_width()));
            ASSERT((coord.y >= 0) && (coord.y < surface->get_height()));
            surface->set_pixel(coord.x, coord.y, this->get_draw_color(patch));
        }
    }
}

void Device::draw(AspngSurface *surface) {
    this->draw_helper(surface, this->all_patches());
}

void Device::draw_debug(AspngSurface *) {}

Rgb Device::get_draw_color(Patch *) {
    return Rgb(0, 0, 0);
}

void Device::click(Coord) {}

void Device::unclick(void) {}
