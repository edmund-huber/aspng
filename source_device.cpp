#include "device.h"

SourceDevice::SourceDevice(void) {
    this->patch = nullptr;
}

SourceDevice::~SourceDevice(void) {
    if (this->patch != nullptr) {
       delete this->patch;
    }
}

Rgb SourceDevice::color = Rgb(0xff, 0xff, 0xff);

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SourceDevice::color);
    return this->patch->size() == 1;
}

Patch *SourceDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

bool SourceDevice::link(Device ***assignments, size_t w, size_t h, std::string *fail_string) {
    Device::find_neighbors(assignments, w, h, &this->neighbors);
    return true;
}
