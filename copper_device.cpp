#include "device.h"

CopperDevice::CopperDevice(void) {
    this->patch = nullptr;
}

CopperDevice::~CopperDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

std::string CopperDevice::name(void) {
    return "Copper";
}

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

Rgb CopperDevice::color = Rgb(0xdb, 0x73, 0);

bool CopperDevice::parse(Png *png, size_t x, size_t y) {
    // TODO: pass in pointer to vector, then can get rid of dynamic allocation of vector
    this->patch = this->parse_flood(png, x, y, CopperDevice::color);
    return this->patch->size() == 1;
}

Patch *CopperDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

bool CopperDevice::link(Device ***assignments, size_t w, size_t h, std::string *fail_string) {
    Device::link_find_neighbors(assignments, w, h, &this->neighbors);
    return true;
}
