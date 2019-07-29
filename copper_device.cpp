#include "device.h"

CopperDevice::CopperDevice(void) {
    this->patch = nullptr;
}

CopperDevice::~CopperDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

Rgb CopperDevice::color = Rgb(0xdb, 0x73, 0);

bool CopperDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, CopperDevice::color);
    return this->patch->size() == 1;
}

Patch *CopperDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}
