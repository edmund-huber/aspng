#ifndef __PORT_H__
#define __PORT_H__

#include <list>
#include <memory>

#include "electrical_value.h"
#include "patch.h"

// At link time, a device determines what the port means to itself, so that
// later, at simulation time, it knows what to do with a port.
enum PortType {
    NoSpecialMeaning,
    ToBeResolved,
    TransistorBridge,
    TransistorGate
};

class Device;

struct PortHalf {
    std::shared_ptr<Device> device;
    Coord coord;
    PortType port_type;
};

class Port {
public:
    Port(std::shared_ptr<Device>, Coord, PortType, std::shared_ptr<Device>, Coord, PortType);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue compute_new_value(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);
    bool is_resolved(void);
    PortHalf *get_our_port_half(Device *);
    PortHalf *get_their_port_half(Device *);

private:
    PortHalf d1_port_half;
    PortHalf d2_port_half;
};

#endif
