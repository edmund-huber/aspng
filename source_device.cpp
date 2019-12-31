#include "device.h"

std::string SourceDevice::name(void) {
    return "Source";
}

Rgb SourceDevice::color = Rgb(0xff, 0xff, 0xff);

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SourceDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch> SourceDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

std::tuple<LinkResult, PortType> SourceDevice::prelink(std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, NoSpecialMeaning);
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning);
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

bool SourceDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> SourceDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue SourceDevice::get_value_at_port(std::shared_ptr<Port>) {
    return HiElectricalValue;
}

void SourceDevice::apply_new_value(Port *, ElectricalValue) {
    // Deliberately empty.
}

Rgb SourceDevice::get_draw_color(void) {
    return SourceDevice::color;
}
