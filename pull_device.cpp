#include "common.h"
#include "device.h"

Rgb PullDevice::yellow = Rgb(0xff, 0xf2, 0);

Device *PullDevice::create(void) {
    return new PullDevice();
}

std::string PullDevice::name(void) {
    return "PullDevice";
}

bool PullDevice::parse(Png *png, size_t x, size_t y) {
    // Let's look for a source or sink pixel.
    this->parse_flood(png, x, y, SourceDevice::color);
    if (this->patch.size() == 0) {
        this->parse_flood(png, x, y, SinkDevice::color);
    }
    // If we found a source or a sink pixel, let's look for the yellow pixel
    // next to it.
    if (this->patch.size() == 1) {
        this->parse_flood_neighbors(png, PullDevice::yellow);
        if (this->patch.size() == 2) {
            return true;
        }
    }
    return false;
}

std::tuple<LinkResult, PortType> PullDevice::prelink(std::shared_ptr<Device> d) {
    ASSERT(0);
    // need to only allow ports on the yellow bit to copper (and touch background)
    // source can only touch background
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, NoSpecialMeaning);
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning);
    return std::make_tuple(LinkError, NoSpecialMeaning);
}

bool PullDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> PullDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue PullDevice::get_value_at_port(std::shared_ptr<Port>) {
    switch (this->pull_type) {
    case PullHi:
        return PullHiElectricalValue;
    case PullLo:
        return PullLoElectricalValue;
    }
    ASSERT(0);
}

void PullDevice::apply_new_value(Port *, ElectricalValue v) {
    // Deliberately empty.
}

Rgb PullDevice::get_draw_color(void) {
    ASSERT(0);
}
