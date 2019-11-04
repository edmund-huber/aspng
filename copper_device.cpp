#include "device.h"

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

std::string CopperDevice::name(void) {
    return "Copper";
}

Rgb CopperDevice::color = Rgb(0xdb, 0x73, 0);

bool CopperDevice::parse(Png *png, size_t x, size_t y) {
    // TODO: pass in pointer to vector, then can get rid of dynamic allocation of vector
    this->patch = this->flood(png, x, y, CopperDevice::color);
    return this->patch.size() == 1;
}

std::vector<Patch> CopperDevice::all_patches(void) {
    std::vector<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

PortType CopperDevice::link(std::shared_ptr<Patch> patch, std::shared_ptr<Device> other) {
    // Copper can connect to anything.
    return CopperPort;
}
