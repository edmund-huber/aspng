#include <algorithm>
#include <iostream>
#include <functional>
#include <list>
#include <map>
#include <memory>

#include "common.h"
#include "device.h"
#include "png.h"

enum BridgeDirections {
    BRIDGE_N = 0,
    BRIDGE_N_E,
    BRIDGE_E,
    BRIDGE_S_E,
    BRIDGE_S,
    BRIDGE_S_W,
    BRIDGE_W,
    BRIDGE_N_W
};

// Bridge is an intermediate data structure that is used by the linker to track
// what Coords might end up being ports between devices. When linking is
// complete, Bridges will be used to construct Ports, and then will be thrown
// away.
struct Bridge {
    std::array<bool, 8> connected;

    Bridge() {
        this->connected.fill(false);
    }
};

typedef std::array<char, 4> Bead;

class BridgeWeldingRule {
public:
    BridgeWeldingRule(Bead _a, Bead _b, Bead _c, Bead _d) : a(_a), b(_b), c(_c), d(_d) {}
    std::list<BridgeWeldingRule> all_variants(void);

private:
    Bead a, b, c, d;
};

// 
std::list<BridgeWeldingRule> BridgeWeldingRule::all_variants(void) {
    std::list<BridgeWeldingRule> list;
    list.push_back(*this);
    // rotated90cw * 4, mirroredx, mirroredy, reassignment
    return list;
}

// BridgeWeldingRules describe how Bridges can be combined to make a larger
// Bridge.
std::list<BridgeWeldingRule> bridge_welding_rules = {
    // Combine trivially-combinable Bridges:
    //  11     11     11     11      11     11     11
    // xo12 , 2o12 , 21o2 , 21ox => xo12 , 2oo2 , 21ox
    // 2222   2x22 , 22x2   2222    2222   2xx2   2222
    //        -----------                  ----
    BridgeWeldingRule(
        Bead({'o', ' ',
              'x', ' '}),
        Bead({' ', 'o',
              ' ', 'x'}),
        Bead({' ', ' ',
              ' ', ' '}),
        Bead({'o', 'o',
              'x', 'x'})
    ),
    // Combine Bridges that need to round a corner to be contiguous:
    //  11     11     11      11     11      11
    // xo12 , 2oo2 , 21ox => xoo2 , 21ox => xoox
    // 2222   2xx2   2222    xxx2   2222    xxxx
    // -----------           ----
    //                       -----------    ----
    BridgeWeldingRule(
        Bead({'x', 'o',
              ' ', ' '}),
        Bead({' ', 'o',
              ' ', 'x'}),
        Bead({' ', ' ',
              'x', ' '}),
        Bead({'x', 'o',
              'x', 'x'})
    )
};

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    std::map<Coord, std::shared_ptr<Device>> device_map;
    std::map<Coord, Bridge> bridge_map;
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

    // Start linking.

    // For every pair of directly (not diagonally) touching pixels between
    // devices, mark them as being bridged. For example:
    //  11      11     11     11     11
    // 2112 => xo12 , 2o12 , 21o2 , 21ox
    // 2222    2222   2x22 , 22x2   2222
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
        }
    }

    // Do passes over bridge_map, attempting to run as many of the
    // BridgeWeldingRules as possible.
    bool did_combine = true;
    int passes = 0;
    while (did_combine) {
        did_combine = false;

        // Have some sensibly high number allowed of passes. Although all the
        // welding passes leave fewer bridges than they started with, if a
        // large number of passes are required, then I feel like I should
        // revisit how this works.
        ASSERT(passes++ < 3);

        for (size_t x = 0; x < png->get_width() - 1; x++) {
            for (size_t y = 0; y < png->get_width() - 1; y++) {
                for (auto &rule : bridge_welding_rules) {
                    for (BridgeWeldingRule &variant : rule.all_variants()) {
                        (void)variant;
                    }
                }
            }
        }

        // TODO: for the moment, this is not doing anything.
    }

    // For the Bridges we figured out, check to see if the Devices allow those
    // Bridges to be there. Allowed Bridges become Ports, or get thrown away.
    // TODO: not actually checking anything atm.


    // In Port debug mode, we don't simulate, we just output one (upscaled)
    // image highlighting all the ports.

    // End of linking. Begin simulation:

    return 0;
}
