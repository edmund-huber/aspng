#include "device.h"

BackgroundDevice::BackgroundDevice(void) {
    this->patch = nullptr;
}

BackgroundDevice::~BackgroundDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Rgb BackgroundDevice::color = Rgb(0x99, 0x99, 0x99);

Device *BackgroundDevice::create(void) {
    return new BackgroundDevice();
}

bool BackgroundDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, BackgroundDevice::color);
    return this->patch->size() >= 1;
}

Patch *BackgroundDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

bool BackgroundDevice::link(Device ***assignments, size_t w, size_t h, std::string *fail_string) {
    return true;
}
