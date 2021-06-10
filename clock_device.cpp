#include "common.h"
#include "device.h"

ClockDevice::ClockDevice(void) {
    this->tick = 0;
}

std::string ClockDevice::name(void) {
    return "ClockDevice";
}

Device *ClockDevice::create(void) {
    return new ClockDevice();
}

std::string ClockDevice::prefix(void) {
    return "clock";
}

bool ClockDevice::sub_parse(AspngSurface *surface, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, std::string param) {
    // Map param to an integer.
    if (param == "") {
        this->divisor = 1;
    } else {
        this->divisor = 0;
        for (size_t i = 0; i < param.length(); i++) {
            this->divisor = (this->divisor * 10) + (param[i] - 'a');
        }
        ASSERT(this->divisor != 0);
    }
    // A valid ClockDevice must be empty (black).
    for (int32_t x = min_x; x <= max_x; x++) {
        for (int32_t y = min_y; y <= max_y; y++) {
            if (surface->get_pixel(x, y) != Rgb(0, 0, 0)) {
                return false;
            }
            this->sub_patch.insert(Coord(x, y));
        }
    }
    return true;
}

bool ClockDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> ClockDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue ClockDevice::get_value_at_port(std::shared_ptr<Port>) {
    return this->get_state();
}

void ClockDevice::apply_new_value(std::shared_ptr<Port>, ElectricalValue) {
    // Deliberately empty.
}

std::list<Patch *> ClockDevice::sub_patches(void) {
    std::list<Patch *> sub_patches;
    sub_patches.push_back(&(this->sub_patch));
    return sub_patches;
}

void ClockDevice::sub_draw(AspngSurface *surface, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    Rgb color;
    switch (this->get_state()) {
    case HiElectricalValue:
        color = SourceDevice::color;
        break;
    case LoElectricalValue:
        color = SinkDevice::color;
        break;
    default:
        ASSERT(0);
    }
    for (int32_t x = min_x; x <= max_x; x++) {
        for (int32_t y = min_y; y <= max_y; y++) {
            surface->set_pixel(x, y, color);
        }
    }
}

void ClockDevice::new_step(void) {
    this->tick++;
}

ElectricalValue ClockDevice::get_state(void) {
    if ((this->tick / this->divisor) % 2 == 0) {
        return HiElectricalValue;
    } else {
        return LoElectricalValue;
    }
}
