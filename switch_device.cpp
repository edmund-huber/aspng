#include "device.h"

std::string SwitchDevice::name(void) {
    return "SwitchDevice";
}

Device *SwitchDevice::create(void) {
    return new SwitchDevice();
}

std::string SwitchDevice::template_name(void) {
    return "switch";
}

// A valid SwitchDevice is white if on, black if off.
bool SwitchDevice::sub_parse(AspngSurface *surface, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    enum {
        On,
        Off,
        Unknown
    } switch_state = Unknown;
    for (int32_t x = min_x; x <= max_x; x++) {
        for (int32_t y = min_y; y <= max_y; y++) {
            Rgb pixel = surface->get_pixel(x, y);
            switch (switch_state) {
            case On:
                if (pixel != Rgb(0xff, 0xff, 0xff)) {
                    return false;
                }
                this->closed = true;
                break;
            case Off:
                if (pixel != Rgb(0, 0, 0)) {
                    return false;
                }
                this->closed = false;
                break;
            case Unknown:
                if (pixel == Rgb(0xff, 0xff, 0xff)) {
                    switch_state = On;
                } else if (pixel == Rgb(0, 0, 0)) {
                    switch_state = Off;
                } else {
                    return false;
                }
                break;
            }
            this->sub_patch.insert(Coord(x, y));
        }
    }
    return switch_state != Unknown;
}

bool SwitchDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> SwitchDevice::propagate(std::shared_ptr<Port> port) {
    if (this->closed) {
        std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
        next_ports.remove(port);
        return next_ports;
    } else {
        std::list<std::shared_ptr<Port>> empty;
        return empty;
    }
}

ElectricalValue SwitchDevice::get_value_at_port(std::shared_ptr<Port>) {
    return EmptyElectricalValue;
}

void SwitchDevice::apply_new_value(std::shared_ptr<Port>, ElectricalValue) {
    // Deliberately empty.
}

std::list<Patch *> SwitchDevice::sub_patches(void) {
    std::list<Patch *> sub_patches;
    sub_patches.push_back(&(this->sub_patch));
    return sub_patches;
}

// White if closed, black otherwise.
void SwitchDevice::sub_draw(AspngSurface *surface, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    Rgb color = this->closed ? Rgb(0xff, 0xff, 0xff) : Rgb(0, 0, 0);
    for (int32_t x = min_x; x <= max_x; x++) {
        for (int32_t y = min_y; y <= max_y; y++) {
            surface->set_pixel(x, y, color);
        }
    }
}

void SwitchDevice::click(Coord) {
    this->closed = !this->closed;
}
