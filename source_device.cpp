#include "device.h"

SourceDevice::SourceDevice(void) {
    this->patch = nullptr;
}

SourceDevice::~SourceDevice(void) {
    if (this->patch != nullptr) {
       delete this->patch;
    }
}

std::string SourceDevice::name(void) {
    return "Source";
}

Rgb SourceDevice::color = Rgb(0xff, 0xff, 0xff);

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood_helper(png, x, y, SourceDevice::color);
    return this->patch->size() == 1;
}

Patch *SourceDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

bool SourceDevice::link(Device ***assignments, size_t w, size_t h, std::string *fail_string) {
    Device::link_find_neighbors(assignments, w, h, &this->neighbors);
    for (auto it = this->neighbors.begin(); it != this->neighbors.end(); it++) {
        if (dynamic_cast<BackgroundDevice *>(*it) != nullptr) continue;
        if (dynamic_cast<CopperDevice *>(*it) != nullptr) continue;
        return false;
    }
    return true;
}
