#include "device.h"

std::string SinkDevice::name(void) {
    return "Sink";
}

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

Rgb SinkDevice::color = Rgb(0x30, 0x30, 0x30);

bool SinkDevice::parse(Png *png, size_t x, size_t y) {
    this->parse_flood(png, x, y, SinkDevice::color);
    return this->patch.size() == 1;
}

std::tuple<LinkResult, PortType> SinkDevice::prelink(std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, NoSpecialMeaning);
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning);
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

bool SinkDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> SinkDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue SinkDevice::get_value_at_port(std::shared_ptr<Port>) {
    return LoElectricalValue;
}

void SinkDevice::apply_new_value(Port *, ElectricalValue) {
    // Deliberately empty.
}

Rgb SinkDevice::get_draw_color(void) {
    return SinkDevice::color;
}
