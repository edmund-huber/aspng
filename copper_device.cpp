#include "device.h"

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

std::string CopperDevice::name(void) {
    return "Copper";
}

Rgb CopperDevice::color = Rgb(0xf8, 0xac, 0x59);

bool CopperDevice::parse(Png *png, size_t x, size_t y) {
    this->parse_flood(png, x, y, CopperDevice::color);
    return this->patch.size() == 1;
}

std::tuple<LinkResult, PortType> CopperDevice::prelink(std::shared_ptr<Device> d) {
    return std::make_tuple(CanLink, NoSpecialMeaning);
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

void CopperDevice::apply_new_value(Port *, ElectricalValue v) {
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

Rgb CopperDevice::get_draw_color(void) {
    return this->color_for_drawing;
}
