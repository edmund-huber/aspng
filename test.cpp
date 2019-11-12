#include <iostream>
#include <functional>
#include <optional>
#include <list>
#include <map>
#include <memory>

#include "common.h"
#include "device.h"
#include "png.h"

// Bridge is an intermediate data structure that is used by the linker to track
// the Patches that will end up being a Port between two devices. When linking
// is complete, the Bridges will have been converted into Ports, and Bridges
// are thrown away.
class Bridge {
    std::shared_ptr<Device> d1;
    Patch p1;
    std::shared_ptr<Device> d2;
    Patch p2;

public:
    Bridge(std::shared_ptr<Device> _d1, Patch _p1, std::shared_ptr<Device> _d2, Patch _p2) : d1(_d1), p1(_p1), d2(_d2), p2(_p2) {}
};

std::shared_ptr<Bridge> single_pixel_bridge(std::map<Coord, std::shared_ptr<Device>> &device_map, Coord coord, size_t x_off, size_t y_off) {
    Coord neighbor = Coord(std::get<0>(coord) + x_off, std::get<1>(coord) + y_off);
    if ((device_map[neighbor] != nullptr) && (device_map[neighbor] != device_map[coord])) {
        Patch p1;
        p1.insert(coord);
        Patch p2;
        p2.insert(neighbor);
        return std::shared_ptr<Bridge>(new Bridge(device_map[coord], p1, device_map[neighbor], p2));
    }
    return nullptr;
}

typedef std::array<char, 4> Weld;

class BridgeWeldingRule {
public:
    BridgeWeldingRule(Weld _a, Weld _b, Weld _c, Weld _d) : a(_a), b(_b), c(_c), d(_d) {}
    BridgeWeldingRule rotated90cw(void);
    BridgeWeldingRule mirroredx(void);
    BridgeWeldingRule mirroredy(void);
    BridgeWeldingRule reassigned(void);

private:
    Weld a, b, c, d;
};

// BridgeWeldingRules describe how Bridges can be combined to make a larger
// Bridge.
std::list<BridgeWeldingRule> bridge_welding_rules = {
    // Combine trivially-combinable Bridges:
    //  11     11     11     11      11     11     11
    // xo12 , 2o12 , 21o2 , 21ox => xo12 , 2oo2 , 21ox
    // 2222   2x22 , 22x2   2222    2222   2xx2   2222
    //        -----------                  ----
    BridgeWeldingRule(
        Weld({'o', 'x',
              ' ', ' '}),
        Weld({' ', ' ',
              'o', 'x'}),
        Weld({' ', ' ',
              ' ', ' '}),
        Weld({'o', 'x',
              'o', 'x'})
    ),
    // Combine Bridges that need to round a corner to be contiguous:
    //  11     11     11      11     11      11
    // xo12 , 2oo2 , 21ox => xoo2 , 21ox => xoox
    // 2222   2xx2   2222    xxx2   2222    xxxx
    // -----------           ----
    //                       -----------    ----
    BridgeWeldingRule(
        Weld({'o', ' ',
              'x', ' '}),
        Weld({'o', 'x',
              ' ', ' '}),
        Weld({' ', ' ',
              ' ', 'x'}),
        Weld({'o', 'x',
              'x', 'x'})
    )
};

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

    // Linking: figure out how devices are touching and set up Ports.

    // Create Bridges for every pair of touching pixels between devices, e.g.:
    //  11      11     11     11     11
    // 2112 => xo12 , 2o12 , 21o2 , 21ox
    // 2222    2222   2x22 , 22x2   2222
    std::map<Coord, std::set<std::shared_ptr<Bridge>>> bridge_map;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            Coord coord(x, y);
            auto bridge = single_pixel_bridge(device_map, coord, 1, 0);
            if (bridge != nullptr) {
                bridge_map[coord].insert(bridge);
            }
            bridge = single_pixel_bridge(device_map, coord, 0, 1);
            if (bridge != nullptr) {
                bridge_map[coord].insert(bridge);
            }
            // Note: only checking 'right' and 'up', because if we also checked
            // 'down' and 'left', we'd end up with duplicate Bridges.
        }
    }

    // Do passes over bridge_map, attempting to run as many of the
    // BridgeWeldingRules as possible.

    // For the Bridges we figured out, check to see if the Devices allow those
    // Bridges to be there.

    // Add Ports to the Devices corresponding to the Bridges. (The Bridges
    // aren't relevant anymore after this.)

    return 0;
}
