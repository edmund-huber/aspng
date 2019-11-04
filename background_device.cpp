#include "device.h"

BackgroundDevice::BackgroundDevice(void) {}

BackgroundDevice::~BackgroundDevice(void) {}

Rgb BackgroundDevice::color = Rgb(0x99, 0x99, 0x99);

Device *BackgroundDevice::create(void) {
    return new BackgroundDevice();
}

std::string BackgroundDevice::name(void) {
    return "Background";
}

bool BackgroundDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, BackgroundDevice::color);
    return this->patch.size() >= 1;
}

std::vector<Patch> BackgroundDevice::all_patches(void) {
    std::vector<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

PortType BackgroundDevice::link(std::shared_ptr<Patch> patch, std::shared_ptr<Device> other) {
    // Backgrounds can connect to anything.
    return BackgroundPort;
}
