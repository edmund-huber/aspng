#include "device.h"

SinkDevice::SinkDevice(void) {
    this->patch = nullptr;
}

SinkDevice::~SinkDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

Rgb SinkDevice::color = Rgb(0, 0, 0);

bool SinkDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SinkDevice::color);
    return this->patch->size() == 1;
}

Patch *SinkDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

bool SinkDevice::link(Device ***assignments, size_t w, size_t h, std::string *fail_string) {
    Device::find_neighbors(assignments, w, h, &this->neighbors);
    for (auto it = this->neighbors.begin(); it != this->neighbors.end(); it++) {
        if (dynamic_cast<BackgroundDevice *>(*it) != nullptr) continue;
        if (dynamic_cast<CopperDevice *>(*it) != nullptr) continue;
        return false;
    }
    return true;
}
