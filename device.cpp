#include "common.h"
#include "device.h"

ElectricalValue combine_electrical_values(ElectricalValue v1, ElectricalValue v2) {
    switch (v1) {
    case EmptyElectricalValue:
        return v2;
    case HiElectricalValue:
        ASSERT(v2 != LoElectricalValue); // TODO error messages
        return HiElectricalValue;
    case LoElectricalValue:
        ASSERT(v2 != HiElectricalValue); // TODO error messages
        return LoElectricalValue;
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
    }
    ASSERT(false);
}

std::list<std::shared_ptr<Port>> Port::propagate(std::shared_ptr<Port> self) {
    std::list<std::shared_ptr<Port>> l;
    l.splice(l.end(), this->d1->propagate(self));
    l.splice(l.end(), this->d2->propagate(self));
    return l;
}

ElectricalValue Port::compute_new_value(std::shared_ptr<Port> self) {
    return combine_electrical_values(
        this->d1->get_value_at_port(self),
        this->d2->get_value_at_port(self)
    );
}

void Port::apply_new_value(ElectricalValue v) {
    this->d1->apply_new_value(v);
    this->d2->apply_new_value(v);
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

void Device::draw(Png *png) {
    auto patches = this->all_patches();
    for (auto i = patches.begin(); i != patches.end(); i++) {
        auto patch = *i;
        for (auto j = patch.begin(); j != patch.end(); j++) {
            auto coord = *j;
            png->set_pixel(std::get<0>(coord), std::get<1>(coord), this->get_draw_color());
        }
    }
}
