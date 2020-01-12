#include <set>

#include "common.h"
#include "device.h"

TransistorDevice::TransistorDevice() {
    this->passing = false;
}

Device *TransistorDevice::create(void) {
    return new TransistorDevice();
}

std::string TransistorDevice::name(void) {
    return "Transistor";
}

Rgb TransistorDevice::color = Rgb(0xff, 0, 0xf2);

bool TransistorDevice::parse(AspngSurface *surface, Coord coord) {
    this->patch = this->flood(surface, coord, TransistorDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch *> TransistorDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch));
    return all_patches;
}

std::tuple<LinkResult, PortType, std::string> TransistorDevice::prelink(Patch *, std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning, "");
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, ToBeResolved, "");
    return std::make_tuple(LinkError, NoSpecialMeaning, "must touch copper or background");
}

bool TransistorDevice::link(void) {
    // For our end of each port, figure out if it's a bridge: bridge ports
    // always lie on the same axis.
    std::set<PortType *> bridges;
    auto all_ports = this->all_ports();
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port1 = *i;
        auto *port1_our_half = port1->get_our_port_half(this);
        ASSERT(port1_our_half->port_type == ToBeResolved);
        auto *port1_their_half = port1->get_their_port_half(this);
        for (auto j = all_ports.begin(); j != all_ports.end(); j++) {
            if (i == j) continue;
            auto port2 = *j;
            auto *port2_their_half = port2->get_their_port_half(this);
            // If the two ports share an axis, then they're bridges.
            if ((port1_their_half->coord.x == port2_their_half->coord.x)
                || (port1_their_half->coord.y == port2_their_half->coord.y)) {
                port1_our_half->port_type = TransistorBridge;
                bridges.insert(&(port1_our_half->port_type));
            }
        }
    }

    // The remaining port must be the gate.
    std::set<PortType *> gates;
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port = *i;
        auto port_our_half = port->get_our_port_half(this);
        if (port_our_half->port_type == ToBeResolved) {
            port_our_half->port_type = TransistorGate;
            gates.insert(&(port_our_half->port_type));
        }
    }

    // We must have exactly 1 gate and 2 bridges.
    return (gates.size() == 1) && (bridges.size() == 2);
}

std::list<std::shared_ptr<Port>> TransistorDevice::propagate(std::shared_ptr<Port> port) {
    auto port_our_half = port->get_our_port_half(this);
    std::list<std::shared_ptr<Port>> empty;
    if (port_our_half->port_type == TransistorGate) {
        return empty;
    } else if (port_our_half->port_type == TransistorBridge) {
        if (this->passing) {
            auto all_ports = this->all_ports();
            std::list<std::shared_ptr<Port>> just_other_bridge;
            for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
                auto port2 = *i;
                auto port2_our_half = port2->get_our_port_half(this);
                if ((port2 != port) && (port2_our_half->port_type == TransistorBridge)) {
                    just_other_bridge.push_back(port2);
                }
            }
            ASSERT(just_other_bridge.size() == 1);
            return just_other_bridge;
        } else {
            return empty;
        }
    }
    ASSERT(0);
}

ElectricalValue TransistorDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void TransistorDevice::apply_new_value(Port *port, ElectricalValue v) {
    auto port_our_half = port->get_our_port_half(this);
    if (port_our_half->port_type == TransistorGate) {
        switch (v) {
        case EmptyElectricalValue:
        case LoElectricalValue:
        case PullLoElectricalValue:
            this->passing = false;
            break;
        case HiElectricalValue:
        case PullHiElectricalValue:
            this->passing = true;
            break;
        }
    }
}

Rgb TransistorDevice::get_draw_color(Patch *) {
    return TransistorDevice::color;
}

void TransistorDevice::draw_debug(AspngSurface *surface) {
    return;
    auto all_ports = this->all_ports();
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port = *i;
        auto port_their_half = port->get_their_port_half(this);
        auto port_our_half = port->get_our_port_half(this);
        if (port_our_half->port_type == TransistorBridge) {
            surface->set_pixel(port_their_half->coord.x, port_their_half->coord.y, Rgb(0, 0, 0xff));
        } else if (port_our_half->port_type == TransistorGate) {
            surface->set_pixel(port_their_half->coord.x, port_their_half->coord.y, Rgb(0xff, 0, 0));
        } else {
            ASSERT(0);
        }
    }
    ASSERT(all_ports.size() == 3);
}
