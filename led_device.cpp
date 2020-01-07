#include "device.h"

LEDDevice::LEDDevice(void) {
    this->active = false;
}

std::string LEDDevice::name(void) {
    return "LEDDevice";
}

Device *LEDDevice::create(void) {
    return new LEDDevice();
}

std::string LEDDevice::template_name(void) {
    return "led";
}

// A valid LEDDevice must be empty (black).
bool LEDDevice::sub_parse(AspngSurface *surface, size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
    for (size_t x = min_x; x <= max_x; x++) {
        for (size_t y = min_y; y <= max_y; y++) {
            if (surface->get_pixel(x, y) != Rgb(0, 0, 0)) {
                return false;
            }
            this->sub_patch.insert(Coord(x, y));
        }
    }
    return true;
}

bool LEDDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> LEDDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> empty;
    return empty;
}

ElectricalValue LEDDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void LEDDevice::apply_new_value(Port *, ElectricalValue v) {
    switch (v) {
    case HiElectricalValue:
    case PullHiElectricalValue:
        this->active = true;
        break;
    case LoElectricalValue:
    case PullLoElectricalValue:
    case EmptyElectricalValue:
        this->active = false;
        break;
    }
}

std::list<Patch *> LEDDevice::sub_patches(void) {
    std::list<Patch *> sub_patches;
    sub_patches.push_back(&(this->sub_patch));
    return sub_patches;
}

// A nice neon green color if active, otherwise dark.
void LEDDevice::sub_draw(AspngSurface *surface, size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
    Rgb color = this->active ? Rgb(0, 0xff, 0) : Rgb(0, 0x30, 0);
    for (size_t x = min_x; x <= max_x; x++) {
        for (size_t y = min_y; y <= max_y; y++) {
            surface->set_pixel(x, y, color);
        }
    }
}
