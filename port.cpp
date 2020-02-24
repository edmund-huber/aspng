#include "aspng.h"
#include "common.h"
#include "device.h"
#include "port.h"

std::list<std::shared_ptr<Port>> Port::propagate(std::shared_ptr<Port> self) {
    std::list<std::shared_ptr<Port>> l;
    l.splice(l.end(), this->d1_port_half.device->propagate(self));
    l.splice(l.end(), this->d2_port_half.device->propagate(self));
    return l;
}

ElectricalValue Port::compute_new_value(std::shared_ptr<Port> self) {
    return combine_electrical_values(
        this->d1_port_half.device->get_value_at_port(self),
        this->d2_port_half.device->get_value_at_port(self)
    );
}

Port::Port(std::shared_ptr<Device> d1, Coord d1_coord, PortType d1_port_type, std::shared_ptr<Device> d2, Coord d2_coord, PortType d2_port_type) {
    this->d1_port_half.device = d1;
    this->d1_port_half.coord = d1_coord;
    this->d1_port_half.port_type = d1_port_type;
    this->d2_port_half.device = d2;
    this->d2_port_half.coord = d2_coord;
    this->d2_port_half.port_type = d2_port_type;
}

void Port::apply_new_value_half(std::shared_ptr<Port> self, ElectricalValue v, PortHalf *port_half) {
    try {
        port_half->device->apply_new_value(self, v);
    } catch (DeviceOperationException &e) {
        throw AspngSimException(
            port_half->device->name() + "::apply_new_value_half: " + e.message,
            port_half->device->get_bounding_box()
        );
    }
}

void Port::apply_new_value(std::shared_ptr<Port> self, ElectricalValue v) {
    this->apply_new_value_half(self, v, &(this->d1_port_half));
    this->apply_new_value_half(self, v, &(this->d2_port_half));
}

bool Port::is_resolved(void) {
    return (this->d1_port_half.port_type != ToBeResolved) && (this->d2_port_half.port_type != ToBeResolved);
}

PortHalf *Port::get_our_port_half(Device *d) {
    if (this->d1_port_half.device.get() == d)
        return &(this->d1_port_half);
    if (this->d2_port_half.device.get() == d)
        return &(this->d2_port_half);
    ASSERT(0);
}

PortHalf *Port::get_their_port_half(Device *d) {
    if (this->d1_port_half.device.get() == d)
        return &(this->d2_port_half);
    if (this->d2_port_half.device.get() == d)
        return &(this->d1_port_half);
    ASSERT(0);
}

void Port::expand_bounding_box(BoundingBox &bounding_box) {
    bounding_box.include(this->d1_port_half.coord);
    bounding_box.include(this->d2_port_half.coord);
}
