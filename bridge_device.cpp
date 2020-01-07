#include "common.h"
#include "device.h"

Device *BridgeDevice::create(void) {
    return new BridgeDevice();
}

std::string BridgeDevice::name(void) {
    return "Bridge";
}

Rgb BridgeDevice::color = Rgb(0x79, 0x44, 0x3b);

bool BridgeDevice::parse(AspngSurface *surface, size_t x, size_t y) {
    this->patch = this->flood(surface, x, y, BridgeDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch *> BridgeDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch));
    return all_patches;
}

std::tuple<LinkResult, PortType> BridgeDevice::prelink(Patch *, std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, ToBeResolved);
    // Why use a Bridge if any of its ports are unused?
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

bool BridgeDevice::link(void) {
    // Why use a Bridge if any of its ports are unused?
    auto all_ports = this->all_ports();
    if (all_ports.size() != 4)
        return false;

    int north_south_ports = 0;
    int east_west_ports = 0;
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port = *i;
        auto *port_our_half = port->get_our_port_half(this);
        ASSERT(port_our_half->port_type == ToBeResolved);
        auto *port_their_half = port->get_their_port_half(this);
        size_t our_x, our_y, their_x, their_y;
        std::tie(our_x, our_y) = port_our_half->coord;
        std::tie(their_x, their_y) = port_their_half->coord;
        if (our_x == their_x) {
            port_our_half->port_type = BridgeNorthSouth;
            north_south_ports++;
        } else if (our_y == their_y) {
            port_our_half->port_type = BridgeEastWest;
            east_west_ports++;
        } else {
            ASSERT(0);
        }
    }
    ASSERT(north_south_ports == 2);
    ASSERT(east_west_ports == 2);

    return true;
}

std::list<std::shared_ptr<Port>> BridgeDevice::propagate(std::shared_ptr<Port> port) {
    // East-west ports connect, and north-south ports connect.
    auto all_ports = this->all_ports();
    std::list<std::shared_ptr<Port>> next_ports;
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port2 = *i;
        if (port2 == port) {
            continue;
        }
        if (port2->get_our_port_half(this)->port_type == port->get_our_port_half(this)->port_type) {
            next_ports.push_back(port2);
        }
    }
    ASSERT(next_ports.size() == 1);
    return next_ports;
}

ElectricalValue BridgeDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void BridgeDevice::apply_new_value(Port *, ElectricalValue v) {
    // Deliberately empty.
}

Rgb BridgeDevice::get_draw_color(Patch *) {
    return BridgeDevice::color;
}
