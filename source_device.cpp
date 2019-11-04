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

std::vector<Patch> SourceDevice::all_patches(void) {
    std::vector<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

PortType SourceDevice::link(std::shared_ptr<Patch> patch, std::shared_ptr<Device> other) {
    // Sources can connect to anything.
    return SourcePort;
}
