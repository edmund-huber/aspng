#include "device.h"

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

CopperDevice::CopperDevice() {
    this->color_for_drawing = CopperDevice::color;
}

std::string CopperDevice::name(void) {
    return "Copper";
}

Rgb CopperDevice::color = Rgb(0xf8, 0xac, 0x59);

bool CopperDevice::parse(AspngSurface *surface, Coord coord) {
    this->patch = this->flood(surface, coord, CopperDevice::color);
    return this->patch.size() >= 1;
}

std::list<Patch *> CopperDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch));
    return all_patches;
}

std::tuple<LinkResult, PortType, std::string> CopperDevice::prelink(Patch *, std::shared_ptr<Device>) {
    return std::make_tuple(CanLink, NoSpecialMeaning, "");
}

bool CopperDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> CopperDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue CopperDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void CopperDevice::apply_new_value(std::shared_ptr<Port>, ElectricalValue v) {
    switch (v) {
    case EmptyElectricalValue:
        this->color_for_drawing = CopperDevice::color;
        break;
    case HiElectricalValue:
    case PullHiElectricalValue:
        this->color_for_drawing = Rgb(0xc0, 0xc0, 0xc0);
        break;
    case LoElectricalValue:
    case PullLoElectricalValue:
        this->color_for_drawing = Rgb(0x60, 0x60, 0x60);
        break;
    }
}

Rgb CopperDevice::get_draw_color(Patch *) {
    return this->color_for_drawing;
}
