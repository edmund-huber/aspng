#include <queue>

#include "aspng.h"
#include "common.h"
#include "net.h"

Net::Net(std::shared_ptr<Port> starting_port, std::set<std::shared_ptr<Port>> &contained_ports) {
    this->new_value = EmptyElectricalValue;
    this->ports_in_net.insert(starting_port);
    ASSERT(contained_ports.find(starting_port) == contained_ports.end());
    contained_ports.insert(starting_port);
    std::queue<std::shared_ptr<Port>> ports_to_visit;
    ports_to_visit.push(starting_port);
    while (!ports_to_visit.empty()) {
        // Take a port off the to_visit queue.
        auto port = ports_to_visit.front();
        ports_to_visit.pop();
        // What (immediate) other ports can be reached through this port?
        auto more_ports_to_visit = port->propagate(port);
        for (auto i = more_ports_to_visit.begin(); i != more_ports_to_visit.end(); i++) {
            auto port2 = *i;
            if (this->ports_in_net.find(port2) == this->ports_in_net.end()) {
                this->ports_in_net.insert(port2);
                ASSERT(contained_ports.find(port2) == contained_ports.end());
                contained_ports.insert(port2);
                ports_to_visit.push(port2);
            }
        }
    }
}

// The value of a net is a function of the values of the individual
// Port-Devices in the net.
void Net::compute_new_value(void) {
    this->new_value = EmptyElectricalValue;
    // For each Port in the Net, figure out the new value and combine that
    // value with what we have so far.
    for (auto i = this->ports_in_net.begin(); i != this->ports_in_net.end(); i++) {
        auto port = *i;
        ElectricalValue v = port->compute_new_value(port);
        try {
            this->new_value = combine_electrical_values(v, this->new_value);
        } catch (ElectricalValueException &e) {
            throw AspngSimException("Net::compute_new_value", this->get_bounding_box());
        }
    }
}

void Net::apply_new_value(void) {
    for (auto i = this->ports_in_net.begin(); i != this->ports_in_net.end(); i++) {
        auto port = *i;
        try {
            port->apply_new_value(port, this->new_value);
        } catch (ElectricalValueException &e) {
            throw AspngSimException("Net::apply_new_value", this->get_bounding_box());
        }
    }
}

bool Net::operator==(const Net &other) const {
    return (this->ports_in_net == other.ports_in_net)
        && (this->new_value == other.new_value);
}

bool Net::operator<(const Net &other) const {
    if (this->ports_in_net < other.ports_in_net) {
        return true;
    }
    if (this->new_value < other.new_value) {
        return true;
    }
    return false;
}

BoundingBox Net::get_bounding_box(void) {
    BoundingBox bounding_box;
    for (auto i = this->ports_in_net.begin(); i != this->ports_in_net.end(); i++) {
        auto port = *i;
        port->expand_bounding_box(bounding_box);
    }
    return bounding_box;
}
