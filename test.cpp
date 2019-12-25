#include <algorithm>
#include <iostream>
#include <functional>
#include <list>
#include <map>
#include <memory>

#include "common.h"
#include "device.h"
#include "png.h"

// If neighboring Coords are different devices, then add ports to both devices
// (to the other device).
void maybe_add_ports(std::map<Coord, std::shared_ptr<Device>> &device_map, Coord coord, size_t x_off, size_t y_off) {
    auto d1 = device_map[coord];
    if (d1 == nullptr)
        return;

    Coord neighbor = Coord(std::get<0>(coord) + x_off, std::get<1>(coord) + y_off);
    auto d2 = device_map[neighbor];
    if ((d2 != nullptr) && (device_map[neighbor] != device_map[coord])) {
        PortType d1_port_type;
        PortType d2_port_type;
        if ((d1->prelink(d2, d2_port_type) == CanLink) && (d2->prelink(d1, d1_port_type) == CanLink)) {
            auto port = std::make_shared<Port>(d2, d1_port_type, d2, d2_port_type);
            d1->add_port(port);
            d2->add_port(port);
            std::cout << "link" << std::endl;
        }
        // TODO: error reporting.
    }
}

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

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
    std::list<std::shared_ptr<Device>> all_devices;
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
                                all_devices.push_back(d);
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
                std::cout << "couldn't parse" << std::endl;
                return -1;
            }
        }
    }

    // Linking: for every pair of directly (not diagonally) touching pixels
    // between devices, add ports.
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            Coord coord(x, y);
            maybe_add_ports(device_map, coord, 1, 0);
            maybe_add_ports(device_map, coord, 0, 1);
            // Note: I'm only considering right and up so that I'm not adding
            // duplicate ports.
        }
    }

    // Simulation! Start by finding the set of all ports (across all devices).
    //

    for (int i = 0; ; i++) {
        auto png = Png::read("tests/basic_source_sink/" + std::to_string(i) + ".png");
        if (png == nullptr)
            break;

        // Until every port has been captured in a net, start a new net from
        // any uncaptured port and propagate.

        // Now calculate the new value of the nets.

        // TODO: flip the clock value
    }

    return 0;
}
