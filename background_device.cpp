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
    this->patch = this->flood(png, x, y, BackgroundDevice::color);
    return this->patch.size() >= 1;
}

std::list<Patch> BackgroundDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

LinkResult BackgroundDevice::prelink(std::shared_ptr<Device> other) {
    return CanTouch;
}
