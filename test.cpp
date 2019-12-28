#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <list>
#include <queue>
#include <map>
#include <memory>
#include <set>

#include "common.h"
#include "device.h"
#include "png.h"

class Net {
public:
    Net();
    Net(std::shared_ptr<Port>);
    bool contains(std::shared_ptr<Port>);
    void compute_new_value(void);
    void apply_new_value(void);

private:
    std::set<std::shared_ptr<Port>> ports_in_net;
    ElectricalValue new_value;
};

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

// If neighboring Coords are different devices, then add ports to both devices
// (pointing to the other device).
void maybe_add_ports(std::map<Coord, std::shared_ptr<Device>> &device_map, Coord coord, size_t x_off, size_t y_off) {
    auto d1 = device_map[coord];
    if (d1 == nullptr)
        return;

    Coord neighbor = Coord(std::get<0>(coord) + x_off, std::get<1>(coord) + y_off);
    auto d2 = device_map[neighbor];
    if ((d2 != nullptr) && (device_map[neighbor] != device_map[coord])) {
        LinkResult d1_link_result, d2_link_result;
        PortType d1_port_type, d2_port_type;
        std::tie(d1_link_result, d2_port_type) = d1->prelink(d2);
        std::tie(d2_link_result, d1_port_type) = d2->prelink(d1);
        if ((d1_link_result == CanLink) && (d2_link_result == CanLink)) {
            auto port = std::make_shared<Port>(d1, d1_port_type, d2, d2_port_type);
            d1->add_port(port);
            d2->add_port(port);
        }
        // TODO: error reporting when LinkError.
    }
}

bool test(std::string path) {
    auto png = Png::read(path + "/_.png");

    std::map<Coord, std::shared_ptr<Device>> device_map;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            device_map[Coord(x, y)] = nullptr;
        }
    }

    std::list<std::function<Device *(void)>> registry;
    registry.push_back(BackgroundDevice::create);
    registry.push_back(CopperDevice::create);
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);

    // Go through all pixels until we find an unassigned pixel ..
    std::set<std::shared_ptr<Device>> all_devices;
    for (size_t y = 0; y < png->get_height(); y++) {
        for (size_t x = 0; x < png->get_width(); x++) {
            if (device_map[Coord(x, y)] == nullptr) {
                // .. and try to parse a device starting from that pixel.
                for (auto it1 = registry.begin(); it1 != registry.end(); it1++) {
                    std::shared_ptr<Device> d((*it1)());
                    if (d->parse(png, x, y)) {
                        // If parsing succeeded, the device claims all those
                        // pixels.
                        auto all_patches = d->all_patches();
                        for (auto it2 = all_patches.begin(); it2 != all_patches.end(); it2++) {
                            Patch patch = *it2;
                            for (auto it3 = patch.begin(); it3 != patch.end(); it3++) {
                                // If this assert fails, it suggests that
                                // greedy parsing rules are overlapping, (they
                                // shouldn't).
                                Coord coord = *it3;
                                ASSERT(device_map[coord] == nullptr);
                                device_map[coord] = d;
                                all_devices.insert(d);
                            }
                        }
                    }
                }
            }
        }
    }

    // After we have passed through the whole image, every pixel should be
    // assigned.
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            if (device_map[Coord(x, y)] == nullptr) {
                // If the parse failed, save the PNG with the recognized pixels
                // masked out, so the user can see what they did wrong.
                for (size_t x = 0; x < png->get_width(); x++) {
                    for (size_t y = 0; y < png->get_height(); y++) {
                        if (device_map[Coord(x, y)] != nullptr) {
                            png->set_pixel(x, y, BackgroundDevice::color);
                        }
                    }
                }
                png->write("fail.png");
                return false;
            }
        }
    }

    // Linking: for every pair of directly (not diagonally) touching pixels
    // between devices, add Ports.
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            Coord coord(x, y);
            maybe_add_ports(device_map, coord, 1, 0);
            maybe_add_ports(device_map, coord, 0, 1);
            // Note: I'm only considering right and up so that I'm not adding
            // duplicate ports.
        }
    }

    bool passed = true;
    for (int i = 0; ; i++) {
        // If there's no further png file, then this test is complete.
        auto png = Png::read("tests/basic_source_sink/" + std::to_string(i) + ".png");
        if (png == nullptr)
            break;

        // Build Nets for simulation: for each port,
        std::set<std::shared_ptr<Port>> all_ports;
        for (auto i = all_devices.begin(); i != all_devices.end(); i++) {
            auto device = *i;
            auto ports = device->all_ports();
            all_ports.insert(ports.begin(), ports.end());
        }
        std::list<std::shared_ptr<Net>> nets;
        for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
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
        for (auto i = all_ports.begin(); i != all_ports.end(); i++) {
            auto port = *i;
            size_t count = 0;
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
                return false;
            }
        }

        // For each net, apply the new value. (These steps are separated so
        // that the new value of one net doesn't affect the new value of the
        // other nets.)
        for (auto i = nets.begin(); i != nets.end(); i++) {
            auto net = *i;
            net->apply_new_value();
        }

        // Store the output image.
        Png out_png(png->get_width(), png->get_height());
        for (auto i = all_devices.begin(); i != all_devices.end(); i++) {
            auto device = *i;
            device->draw(&out_png);
        }
        out_png.write("/tmp/output.png"); // TODO write somewhere more obvious (in directory)

        // Compare the output image to the expected image.
        ASSERT(out_png.get_width() == png->get_width());
        ASSERT(out_png.get_height() == png->get_height());
        for (size_t x = 0; x < out_png.get_width(); x++) {
            for (size_t y = 0; y < out_png.get_height(); y++) {
                if (out_png.get_pixel(x, y) != png->get_pixel(x, y)) {
                    passed = false;
                }
            }
        }
        // TODO: flip the clock value
    }

    return passed;
}

int main(void) {
    // Run all tests under tests/.
    int failures = 0;
    for (auto &entry : std::filesystem::directory_iterator("tests/")) {
        bool passed = test(entry.path());
        bool expect_fail = std::filesystem::exists(entry.path().string() + "/expect_fail");
        std::string status;
        if (passed) {
            status = "PASS";
        } else if (!passed && expect_fail) {
            status = "PAIL";
        } else {
            status = "FAIL";
            failures++;
        }
        std::cout << status << " " << entry << std::endl;
    }
    if (failures == 0) {
        return 0;
    } else {
        return 1;
    }
}
