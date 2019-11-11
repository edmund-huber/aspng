#include <iostream>
#include <functional>
#include <list>
#include <memory>

#include "common.h"
#include "device.h"
#include "png.h"

class Port {
    Device *d1;
    Patch p1;
    Device *d2;
    Patch p2;

public:
    Port(Device *_d1, Patch _p1, Device *_d2, Patch _p2) : d1(_d1), p1(_p1), d2(_d2), p2(_p2) {}
};

void single_pixel_port_helper(Device *d1, Patch &p1, Coord coord, Device *d2, Patch &p2, size_t x_off, size_t y_off, std::list<Port> &ports) {
    Coord neighbor = Coord(std::get<0>(coord) + x_off, std::get<1>(coord) + y_off);
    if (p2.find(neighbor) != p2.end()) {
        Patch port_p1;
        port_p1.insert(coord);
        Patch port_p2;
        port_p2.insert(neighbor);
        ports.push_back(Port(d1, port_p1, d2, port_p2));
    }
}

void patches_to_ports(Device *d1, Patch &p1, Device *d2, Patch &p2) {
    // Pass 1: for each pixel in p1, if any pixel in patch2 touches, those
    // are one port.
    std::list<Port> ports;
    for (auto it = p1.begin(); it != p1.end(); it++) {
        Coord coord = *it;
        single_pixel_port_helper(d1, p1, coord, d2, p2, 1, 0, ports);
        single_pixel_port_helper(d1, p1, coord, d2, p2, -1, 0, ports);
        single_pixel_port_helper(d1, p1, coord, d2, p2, 0, 1, ports);
        single_pixel_port_helper(d1, p1, coord, d2, p2, 0, -1, ports);
    }

    // Now we have a bunch of one-pixel ports. While we can combine ports
    // (because they are right next to each other and are actually one port),
    // do so.

    // for each port1 and port2
    //     if some pixel1 and pixel2 in port1.a and port2.a touch,
    //         (depending on how those 2 touch, if same for .b ..)
    //             merge port1 and port2
    // (repeat until did a pass with no merges)


    // .. now have to do something to give each device its ports..
}

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    Device ***device_map = new Device**[png->get_width()];
    for (size_t x = 0; x < png->get_width(); x++) {
        device_map[x] = new Device*[png->get_height()];
        for (size_t y = 0; y < png->get_height(); y++) {
            device_map[x][y] = nullptr;
        }
    }

    std::list<std::function<Device *(void)>> registry;
    registry.push_back(BackgroundDevice::create);
    registry.push_back(CopperDevice::create);
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);

    // Go through all pixels until we find an unassigned pixel ..
    std::list<Device *> all_devices;
    for (size_t y = 0; y < png->get_height(); y++) {
        for (size_t x = 0; x < png->get_width(); x++) {
            if (device_map[x][y] == nullptr) {
                // .. and try to parse a device starting from that pixel.
                for (auto it1 = registry.begin(); it1 != registry.end(); it1++) {
                    Device *d = (*it1)();
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
                                ASSERT(device_map[std::get<0>(coord)][std::get<1>(coord)] == nullptr)
                                device_map[std::get<0>(coord)][std::get<1>(coord)] = d;
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
            if (device_map[x][y] == nullptr) {
                // If the parse failed, save the PNG with the recognized pixels
                // masked out, so the user can see what they did wrong.
                for (size_t x = 0; x < png->get_width(); x++) {
                    for (size_t y = 0; y < png->get_height(); y++) {
                        if (device_map[x][y] != nullptr) {
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

    // Linking: figure out how devices are touching and set up ports.
    std::list<Port> ports;
    for (auto it1 = all_devices.begin(); it1 != all_devices.end(); it1++) {
        // For each pair of devices d1 and d2,
        Device *d1 = *it1;
        for (auto it2 = all_devices.begin(); it2 != all_devices.end(); it2++) {
            Device *d2 = *it2;
            // .. and each pair of their constituent patches,
            auto d1_all_patches = d1->all_patches();
            for (auto it3 = d1_all_patches.begin(); it3 != d1_all_patches.end(); it3++) {
                Patch patch1 = *it3;
                auto d2_all_patches = d2->all_patches();
                for (auto it4 = d2_all_patches.begin(); it4 != d2_all_patches.end(); it4++) {
                    Patch patch2 = *it4;
                    // .. find the longest contiguous patches of d1 and d2
                    // which touch at every pixel.
                    patches_to_ports(d1, patch1, d2, patch2);
                }
            }
        }
    }

    return 0;
}
