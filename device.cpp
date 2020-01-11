#include "common.h"
#include "device.h"

ElectricalValue combine_electrical_values(ElectricalValue v1, ElectricalValue v2) {
    switch (v1) {
    case EmptyElectricalValue:
        return v2;
    case HiElectricalValue:
        if (v2 == LoElectricalValue) {
            throw ElectricalValueException();
        }
        return HiElectricalValue;
    case LoElectricalValue:
        if (v2 == HiElectricalValue) {
            throw ElectricalValueException();
        }
        return LoElectricalValue;
    case PullLoElectricalValue:
        if (v2 == HiElectricalValue) {
            return HiElectricalValue;
        } else {
            return PullLoElectricalValue;
        }
    case PullHiElectricalValue:
        if (v2 == LoElectricalValue) {
            return LoElectricalValue;
        } else {
            return PullHiElectricalValue;
        }
    }
    ASSERT(false);
}

std::string electrical_value_to_str(ElectricalValue v) {
    switch (v) {
    case EmptyElectricalValue:
        return "Empty";
    case HiElectricalValue:
        return "Hi";
    case LoElectricalValue:
        return "Lo";
    case PullHiElectricalValue:
        return "PullHi";
    case PullLoElectricalValue:
        return "PullLo";
    }
    ASSERT(false);
}

void Device::add_port(std::shared_ptr<Port> p) {
    this->ports.push_back(p);
}

std::list<std::shared_ptr<Port>> Device::all_ports(void) {
    return this->ports;
}

Patch Device::flood(AspngSurface *surface, int32_t x, int32_t y, Rgb color) {
    Patch patch, visited;
    Device::flood_helper(surface, x, y, color, patch, visited);
    return patch;
}

void Device::flood_helper(AspngSurface *surface, int32_t x, int32_t y, Rgb color, Patch &patch, Patch &visited) {
    if ((x < 0) || (x >= surface->get_width())) {
        return;
    }
    if ((y < 0) || (y >= surface->get_height())) {
        return;
    }
    if (visited.find(Coord(x, y)) != visited.end()) {
        return;
    }
    visited.insert(Coord(x, y));
    if (surface->get_pixel(x, y) == color) {
        patch.insert(Coord(x, y));
        Device::flood_helper(surface, x - 1, y, color, patch, visited);
        Device::flood_helper(surface, x + 1, y, color, patch, visited);
        Device::flood_helper(surface, x, y - 1, color, patch, visited);
        Device::flood_helper(surface, x, y + 1, color, patch, visited);
    }
}

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
