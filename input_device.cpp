#include "device.h"

InputDevice::InputDevice(void) {
    this->being_clicked = false;
}

std::string InputDevice::name(void) {
    return "InputDevice";
}

Device *InputDevice::create(void) {
    return new InputDevice();
}

// A valid InputDevice must be empty (black).
bool InputDevice::sub_parse(AspngSurface *surface, size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
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

bool InputDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> InputDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue InputDevice::get_value_at_port(std::shared_ptr<Port>) {
    if (this->being_clicked) {
        return HiElectricalValue;
    } else {
        return LoElectricalValue;
    }
}

void InputDevice::apply_new_value(Port *, ElectricalValue) {
    // Deliberately empty.
}

std::list<Patch *> InputDevice::sub_patches(void) {
    std::list<Patch *> sub_patches;
    sub_patches.push_back(&(this->sub_patch));
    return sub_patches;
}

// "Source color" if being clicked, "sink color" otherwise.
void InputDevice::sub_draw(AspngSurface *surface, size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
    Rgb color = this->being_clicked ? SourceDevice::color : SinkDevice::color;
    for (size_t x = min_x; x <= max_x; x++) {
        for (size_t y = min_y; y <= max_y; y++) {
            surface->set_pixel(x, y, color);
        }
    }
}
