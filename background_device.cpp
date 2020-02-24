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

bool BackgroundDevice::parse(AspngSurface *surface, Coord coord) {
    this->patch = this->flood(surface, coord, BackgroundDevice::color);
    return this->patch.size() >= 1;
}

std::list<Patch *> BackgroundDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch));
    return all_patches;
}

std::tuple<LinkResult, PortType, std::string> BackgroundDevice::prelink(Patch *, std::shared_ptr<Device> d) {
    return std::make_tuple(CanTouch, NoSpecialMeaning, "");
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

void BackgroundDevice::apply_new_value(std::shared_ptr<Port>, ElectricalValue) {
    // Deliberately empty.
}

Rgb BackgroundDevice::get_draw_color(Patch *) {
    return BackgroundDevice::color;
}
