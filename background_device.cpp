#include "device.h"

BackgroundDevice::BackgroundDevice(void) {}

BackgroundDevice::~BackgroundDevice(void) {}

Rgb BackgroundDevice::color = Rgb(0, 0, 0);

Device *BackgroundDevice::create(void) {
    return new BackgroundDevice();
}

std::string BackgroundDevice::name(void) {
    return "Background";
}

bool BackgroundDevice::parse(Png *png, size_t x, size_t y) {
    this->parse_flood(png, x, y, BackgroundDevice::color);
    return this->patch.size() >= 1;
}

std::tuple<LinkResult, PortType> BackgroundDevice::prelink(std::shared_ptr<Device> d) {
    return std::make_tuple(CanTouch, NoSpecialMeaning);
}

bool BackgroundDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> BackgroundDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> empty;
    return empty;
}

ElectricalValue BackgroundDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void BackgroundDevice::apply_new_value(Port *, ElectricalValue) {
    // Deliberately empty.
}

Rgb BackgroundDevice::get_draw_color(void) {
    return BackgroundDevice::color;
}
