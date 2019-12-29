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

bool TransistorDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, TransistorDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch> TransistorDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

std::tuple<LinkResult, PortType> TransistorDevice::prelink(std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning);
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, ToBeResolved);
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

bool TransistorDevice::link(void) {
    // For our end of each port, figure out if it's a bridge: bridge ports
    // always lie on the same axis.
    int bridge_count = 0;
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
            size_t c1_x, c1_y, c2_x, c2_y;
            std::tie(c1_x, c1_y) = port1_their_half->coord;
            std::tie(c2_x, c2_y) = port2_their_half->coord;
            if ((c1_x == c2_x) || (c1_y == c2_y)) {
                port1_our_half->port_type = TransistorBridge;
                bridge_count++;
            }
        }
    }

    // The remaining port must be the gate.
    int gate_count = 0;
    for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
        auto port = *i;
        auto port_our_half = port->get_our_port_half(this);
        if (port_our_half->port_type == ToBeResolved) {
            port_our_half->port_type = TransistorGate;
            gate_count++;
        }
    }

    // We must have exactly 1 gate and 2 bridges.
    return (gate_count == 1) && (bridge_count == 2);
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

void TransistorDevice::apply_new_value(ElectricalValue v) {
    switch (v) {
    case EmptyElectricalValue:
    case LoElectricalValue:
        this->passing = false;
        break;
    case HiElectricalValue:
        this->passing = true;
        break;
    }
}

Rgb TransistorDevice::get_draw_color(void) {
    return TransistorDevice::color;
}
