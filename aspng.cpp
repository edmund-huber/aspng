#include <functional>
#include <memory>

#include "aspng.h"
#include "common.h"
#include "device.h"
#include "net.h"

Aspng::Aspng(AspngSurface *surface, std::string &error) {
    std::map<Coord, std::shared_ptr<Device>> device_map;
    for (int32_t x = 0; x < surface->get_width(); x++) {
        for (int32_t y = 0; y < surface->get_height(); y++) {
            device_map[Coord(x, y)] = nullptr;
        }
    }

    std::list<std::function<Device *(void)>> registry;
    registry.push_back(BackgroundDevice::create);
    registry.push_back(BridgeDevice::create);
    registry.push_back(CopperDevice::create);
    registry.push_back(InputDevice::create);
    registry.push_back(LEDDevice::create);
    registry.push_back(PullDevice::create);
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);
    registry.push_back(SwitchDevice::create);
    registry.push_back(TransistorDevice::create);

    // Go through all pixels until we find an unassigned pixel ..
    for (int32_t y = 0; y < surface->get_height(); y++) {
        for (int32_t x = 0; x < surface->get_width(); x++) {
            if (device_map[Coord(x, y)] == nullptr) {
                // .. and try to parse a device starting from that pixel.
                for (auto it1 = registry.begin(); it1 != registry.end(); it1++) {
                    std::shared_ptr<Device> d((*it1)());
                    if (d->parse(surface, Coord(x, y))) {
                        bool d_is_valid = true;

                        // Lets find all pixels where we have a parse conflict.
                        std::set<std::shared_ptr<Device>> conflicting_devices;
                        auto d_combined = d->all_patches_combined();
                        for (auto it2 = d_combined.begin(); it2 != d_combined.end(); it2++) {
                            Coord coord = *it2;
                            if (device_map[coord] != nullptr) {
                                conflicting_devices.insert(device_map[coord]);
                            }
                        }

                        // If any of the pixels are owned by some other
                        // devices, then this device needs to contain all the
                        // other devices' pixels (or vice versa), otherwise
                        // it's a parsing error by our standards.
                        if (conflicting_devices.size() > 0) {
                            for (auto it2 = conflicting_devices.begin(); it2 != conflicting_devices.end(); it2++) {
                                auto other = *it2;
                                auto other_combined = other->all_patches_combined();
                                if (d_combined.is_subset(other_combined)) {
                                    // We won't continue with `d`.
                                    d_is_valid = false;
                                    break;
                                } else if (other_combined.is_subset(d_combined)) {
                                    // The other parse isn't maximal, remove it.
                                    ASSERT(this->all_devices.erase(other) == 1);
                                } else {
                                    error = "incompletable parse";
                                    return;
                                }
                            }
                        }
                        // To complete the parse, the device should claim all
                        // of its pixels.
                        if (d_is_valid) {
                            for (auto it2 = d_combined.begin(); it2 != d_combined.end(); it2++) {
                                Coord coord = *it2;
                                device_map[coord] = d;
                            }
                            this->all_devices.insert(d);
                        }
                    }
                }
            }
        }
    }

    // After we have passed through the whole image, every pixel should be
    // assigned.
    for (int32_t x = 0; x < surface->get_width(); x++) {
        for (int32_t y = 0; y < surface->get_height(); y++) {
            if (device_map[Coord(x, y)] == nullptr) {
                error = "incomplete parse";
                return;
            }
        }
    }

    // Pre-linking: for every pair of directly (not diagonally) touching pixels
    // between devices, add Ports (or not).
    for (int32_t x = 0; x < surface->get_width(); x++) {
        for (int32_t y = 0; y < surface->get_height(); y++) { // TODO .. should be -1 (ditto above)?
            Coord coord(x, y);
            std::string prelink_fail;
            if ((prelink_fail = maybe_add_ports(device_map, coord, 1, 0)) != "") {
                error = "prelink fail: " + prelink_fail;
                return;
            }
            if ((prelink_fail = maybe_add_ports(device_map, coord, 0, 1)) != "") {
                error = "prelink fail: " + prelink_fail;
                return;
            }
            // Note: I'm only considering right and up so that I'm not adding
            // duplicate ports.
        }
    }

    // Link all devices.
    for (auto i = this->all_devices.begin(); i != this->all_devices.end(); i++) {
        auto device = *i;
        if ((error = device->link()) != "") {
            auto all_patches_combined = device->all_patches_combined();
            ASSERT(all_patches_combined.size() > 0);
            auto coord = *(all_patches_combined.begin());
            error = "link fail: " + device->name() + " " + coord + " - " + error;
            return;
        }
    }

    // Sanity check: no Ports remain in the ToBeResolved state.
    for (auto i = this->all_devices.begin(); i != this->all_devices.end(); i++) {
        auto device = *i;
        auto ports = device->all_ports();
        for (auto j = ports.begin(); j != ports.end(); j++) {
            auto port = *j;
            if (!port->is_resolved()) {
                error = "unresolved port";
                return;
            }
            this->all_ports.insert(port);
        }
    }
}

// If neighboring Coords are different devices, then add ports to both devices
// (pointing to the other device).
std::string Aspng::maybe_add_ports(std::map<Coord, std::shared_ptr<Device>> &device_map, Coord d1_coord, int32_t x_off, int32_t y_off) {
    auto d1 = device_map[d1_coord];
    if (d1 == nullptr)
        return "";

    Coord d2_coord = Coord(d1_coord.x + x_off, d1_coord.y + y_off);
    auto d2 = device_map[d2_coord];
    if ((d2 != nullptr) && (d2 != d1)) {
        LinkResult d1_link_result, d2_link_result;
        PortType d1_port_type, d2_port_type;
        std::string error;
        auto d1_patch = d1->find_patch_containing(d1_coord);
        std::tie(d1_link_result, d1_port_type, error) = d1->prelink(d1_patch, d2);
        if (d1_link_result == LinkError) {
            return d1->name() + " " + d1_coord + " - " + error;
        }
        auto d2_patch = d2->find_patch_containing(d2_coord);
        std::tie(d2_link_result, d2_port_type, error) = d2->prelink(d2_patch, d1);
        if (d2_link_result == LinkError) {
            return d2->name() + " " + d2_coord + " - " + error;
        }
        if ((d1_link_result == CanLink) && (d2_link_result == CanLink)) {
            auto port = std::make_shared<Port>(d1, d1_coord, d1_port_type, d2, d2_coord, d2_port_type);
            d1->add_port(port);
            d2->add_port(port);
        }
    }

    return "";
}

std::string Aspng::step(void) {
    // Build Nets for simulation: for each port,
    std::list<std::shared_ptr<Net>> nets;
    for (auto i = this->all_ports.begin(); i != this->all_ports.end(); i++) {
        // .. see if the Port is in a Net already.
        auto port = *i;
        bool found = false;
        for (auto j = nets.begin(); j != nets.end(); j++) {
            auto net = *j;
            if (net->contains(port)) {
                found = true;
                break;
            }
        }
        // If this Port isn't in any Net, then let's start a new Net,
        // propagating out from this Port.
        if (!found) {
            nets.push_back(std::make_shared<Net>(port));
        }
    }

    // Sanity check: every Port should be contained in exactly one Net.
    for (auto i = this->all_ports.begin(); i != this->all_ports.end(); i++) {
        auto port = *i;
        int32_t count = 0;
        for (auto j = nets.begin(); j != nets.end(); j++) {
            auto net = *j;
            if (net->contains(port)) {
                count++;
            }
        }
        ASSERT(count > 0);
        ASSERT(count == 1);
    }

    // For each net, compute the new value.
    for (auto i = nets.begin(); i != nets.end(); i++) {
        auto net = *i;
        try {
            net->compute_new_value();
        } catch (ElectricalValueException &e) {
            return "bad electrical value";
        }
    }

    // For each net, apply the new value. (These steps are separated so that
    // the new value of one net doesn't affect the new value of the other
    // nets.)
    for (auto i = nets.begin(); i != nets.end(); i++) {
        auto net = *i;
        net->apply_new_value();
    }

    return "";
}

void Aspng::draw(AspngSurface *surface) {
    for (auto i = this->all_devices.begin(); i != this->all_devices.end(); i++) {
        auto device = *i;
        device->draw(surface);
    }
    for (auto i = this->all_devices.begin(); i != this->all_devices.end(); i++) {
        auto device = *i;
        device->draw_debug(surface);
    }
}

std::shared_ptr<Device> Aspng::which_device(Coord coord) {
    for (auto i = this->all_devices.begin(); i != this->all_devices.end(); i++) {
        auto device = *i;
        auto all_patches = device->all_patches();
        for (auto j = all_patches.begin(); j != all_patches.end(); j++) {
            auto patch = *j;
            if (patch->find(coord) != patch->end()) {
                return device;
            }
        }
    }
    return nullptr;
}
