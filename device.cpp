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

std::list<std::shared_ptr<Port>> Port::propagate(std::shared_ptr<Port> self) {
    std::list<std::shared_ptr<Port>> l;
    l.splice(l.end(), this->d1_port_half.device->propagate(self));
    l.splice(l.end(), this->d2_port_half.device->propagate(self));
    return l;
}

ElectricalValue Port::compute_new_value(std::shared_ptr<Port> self) {
    return combine_electrical_values(
        this->d1_port_half.device->get_value_at_port(self),
        this->d2_port_half.device->get_value_at_port(self)
    );
}

Port::Port(std::shared_ptr<Device> d1, Coord d1_coord, PortType d1_port_type, std::shared_ptr<Device> d2, Coord d2_coord, PortType d2_port_type) {
    d1_port_half.device = d1;
    d1_port_half.coord = d1_coord;
    d1_port_half.port_type = d1_port_type;
    d2_port_half.device = d2;
    d2_port_half.coord = d2_coord;
    d2_port_half.port_type = d2_port_type;
}

void Port::apply_new_value(ElectricalValue v) {
    this->d1_port_half.device->apply_new_value(this, v);
    this->d2_port_half.device->apply_new_value(this, v);
}

bool Port::is_resolved(void) {
    return (this->d1_port_half.port_type != ToBeResolved) && (this->d2_port_half.port_type != ToBeResolved);
}

PortHalf *Port::get_our_port_half(Device *d) {
    if (this->d1_port_half.device.get() == d)
        return &(this->d1_port_half);
    if (this->d2_port_half.device.get() == d)
        return &(this->d2_port_half);
    ASSERT(0);
}

PortHalf *Port::get_their_port_half(Device *d) {
    if (this->d1_port_half.device.get() == d)
        return &(this->d2_port_half);
    if (this->d2_port_half.device.get() == d)
        return &(this->d1_port_half);
    ASSERT(0);
}

void Device::add_port(std::shared_ptr<Port> p) {
    this->ports.push_back(p);
}

std::list<std::shared_ptr<Port>> Device::all_ports(void) {
    return this->ports;
}

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
    if (visited.find(Coord(x, y)) != visited.end()) {
        return;
    }
    visited.insert(Coord(x, y));
    if (png->get_pixel(x, y) == color) {
        patch.insert(Coord(x, y));
        Device::flood_helper(png, x - 1, y, color, patch, visited);
        Device::flood_helper(png, x + 1, y, color, patch, visited);
        Device::flood_helper(png, x, y - 1, color, patch, visited);
        Device::flood_helper(png, x, y + 1, color, patch, visited);
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

void Device::draw(Png *png) {
    auto patches = this->all_patches();
    for (auto i = patches.begin(); i != patches.end(); i++) {
        auto patch = *i;
        for (auto j = patch->begin(); j != patch->end(); j++) {
            auto coord = *j;
            size_t x, y;
            std::tie(x, y) = coord;
            ASSERT((x >= 0) && (x < png->get_width()));
            ASSERT((y >= 0) && (y < png->get_height()));
            png->set_pixel(x, y, this->get_draw_color(patch));
        }
    }
}

void Device::draw_debug(Png *png) {}
