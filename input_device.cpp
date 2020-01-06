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
