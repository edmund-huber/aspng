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

std::list<Patch> SourceDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

LinkResult SourceDevice::prelink(std::shared_ptr<Device> other) {
    if (std::dynamic_pointer_cast<CopperDevice>(other))
        return CanLink;
    if (std::dynamic_pointer_cast<BackgroundDevice>(other))
        return CanTouch;
    return LinkError;
}
