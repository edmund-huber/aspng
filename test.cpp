#include <iostream>
#include <functional>
#include <memory>
#include <vector>

#include "common.h"
#include "device.h"
#include "png.h"

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    Device ***device_map = new Device**[png->get_width()];
    for (size_t x = 0; x < png->get_width(); x++) {
        device_map[x] = new Device*[png->get_height()];
        for (size_t y = 0; y < png->get_height(); y++) {
            device_map[x][y] = nullptr;
        }
    }

    std::vector<std::function<Device *(void)>> registry;
    registry.push_back(BackgroundDevice::create);
    registry.push_back(CopperDevice::create);
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);

    // Go through all pixels until we find an unassigned pixel ..
    std::vector<Device *> all_devices;
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
                                Coord2d coord2d = *it3;
                                ASSERT(device_map[coord2d.x][coord2d.y] == nullptr)
                                device_map[coord2d.x][coord2d.y] = d;
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
    for (auto it1 = all_devices.begin(); it1 != all_devices.end(); it1++) {
        // For each pair of devices d1 and d2,
        Device *d1 = *it1;
        for (auto it2 = all_devices.begin(); it2 != all_devices.end(); it2++) {
            Device *d2 = *it2;
            // and each pair of their constituent patches,
            auto d1_all_patches = d1->all_patches();
            for (auto it3 = d1_all_patches.begin(); it3 != d1_all_patches.end(); it3++) {
                Patch patch1 = *it3;
                auto d2_all_patches = d2->all_patches();
                for (auto it4 = d2_all_patches.begin(); it4 != d2_all_patches.end(); it4++) {
                    // find the longest contiguous patches of d1 and d2 which
                    // touch at every pixel.
                    Patch patch2 = *it4;
                }
            }
        }
    }
 
    /*for each pair of devices d1 and d2, and each pair of their
    // constituent patches (all_patches), find the longest contiguous patches
    // of d1 and d2 which touch at every pixel.

    
    
    from each pixel, find the longest pair of touching patches in
    // separate devices. If both devices agree, then the pair comprises a port.
    // If they don't agree, it's an error.
    bool link_error = false;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            std::shared_ptr<Patch> patch1, patch2;
            std::string error_str;
            auto ret = attempt_port(device_map, x, y, png->get_width(), png->get_height(), patch1, patch2, error_str);
            if (ret == AttemptPortSuccess) {
                // TODO: set up a port w/ patch1 and patch2, give it to 
                // device1->port1 and device2->port2
            } else if (ret == AttemptPortFailure) {
                std::cout << "couldn't link: " << error_str << std::endl;
                link_error = true;
            }
        }
    }
    if (link_error) {
        return 1;
    }*/

    // TODO: delete each device in all_devices (delete other stuff?)
    // TODO: move some/all of this into a Parser class

    return 0;
}
