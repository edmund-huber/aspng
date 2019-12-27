#include "device.h"

std::string SinkDevice::name(void) {
    return "Sink";
}

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

Rgb SinkDevice::color = Rgb(0x59, 0x59, 0x59);

bool SinkDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SinkDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch> SinkDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

std::tuple<LinkResult, PortType> SinkDevice::prelink(std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, NoSpecialMeaning);
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning);
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

std::list<std::shared_ptr<Port>> SinkDevice::propagate(Port *port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(std::shared_ptr<Port>(port));
    return next_ports;
}
