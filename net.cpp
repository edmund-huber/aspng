#include <queue>

#include "net.h"

Net::Net(std::shared_ptr<Port> p) {
    // Initialize class members.
    this->ports_in_net.insert(p);
    this->new_value = EmptyElectricalValue;

    std::queue<std::shared_ptr<Port>> ports_to_visit;
    ports_to_visit.push(p);
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
                ports_to_visit.push(port2);
            }
        }
    }
}

bool Net::contains(std::shared_ptr<Port> port) {
    return this->ports_in_net.find(port) != this->ports_in_net.end();
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
        this->new_value = combine_electrical_values(v, this->new_value);
    }
}

void Net::apply_new_value(void) {
    for (auto i = this->ports_in_net.begin(); i != this->ports_in_net.end(); i++) {
        auto port = *i;
        port->apply_new_value(this->new_value);
    }
}
