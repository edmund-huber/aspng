#include "device.h"

std::string SinkDevice::name(void) {
    return "Sink";
}

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

Rgb SinkDevice::color = Rgb(0x59, 0x59, 0x59);

bool SinkDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SinkDevice::color);
    return this->patch.size() == 1;
}

std::list<Patch> SinkDevice::all_patches(void) {
    std::list<Patch> all_patches;
    all_patches.push_back(this->patch);
    return all_patches;
}

LinkResult SinkDevice::prelink(std::shared_ptr<Device> other) {
    if (std::dynamic_pointer_cast<CopperDevice>(other))
        return CanLink;
    if (std::dynamic_pointer_cast<BackgroundDevice>(other))
        return CanTouch;
    return LinkError;
}
