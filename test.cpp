#include <iostream>
#include <functional>
#include <vector>

#include "common.h"
#include "device.h"
#include "png.h"

void net_segment(Device *root, Device *prev_d, Device *d) {
    // If this Device is already in this net, we can stop.
    if (d->head_of_net == root) return;

    // If this Device is in another net, then something has gone horribly
    // wrong.
    ASSERT(d->head_of_net == nullptr)
    // this is all wrong. a device (e.g. a transistor) can be in multiple nets.

    // This device is in the current net.
    d->head_of_net = root;
    d->next_in_net = root->next_in_net;
    root->next_in_net = d;

    // Also add all of the devices that can be reached through this device,
    // from the previous device.
    auto connected = d->connected_devices(prev_d);
    for (auto iter = connected.begin(); iter != connected.end(); iter++) {
        net_segment(root, d, *iter);
    }
}

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    Device ***assigned = new Device**[png->get_width()];
    for (size_t x = 0; x < png->get_width(); x++) {
        assigned[x] = new Device*[png->get_height()];
        for (size_t y = 0; y < png->get_height(); y++) {
            assigned[x][y] = nullptr;
        }
    }

    std::vector<std::function<Device *(void)>> registry;
    registry.push_back(BackgroundDevice::create);
    registry.push_back(CopperDevice::create);
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);

    std::vector<Device *> all_devices;

    // Go through all pixels until we find an unassigned pixel, and try to
    // parse a device starting from there.
    for (size_t y = 0; y < png->get_height(); y++) {
        for (size_t x = 0; x < png->get_width(); x++) {
            if (assigned[x][y] == nullptr) {
                for (auto iter = registry.begin(); iter != registry.end(); iter++) {
                    Device *d = (*iter)();
                    if (d->parse(png, x, y)) {
                        // If parsing succeeded, we'll assign all the pixels
                        // that the device parsed out.
                        Patch *all_patches = d->all_patches();
                        for (auto iter2 = all_patches->begin(); iter2 != all_patches->end(); iter2++) {
                            Coord2d xy = *iter2;
                            // If this assert fails, it suggests that greedy
                            // parsing rules are overlapping, (they shouldn't).
                            ASSERT(assigned[xy.x][xy.y] == nullptr)
                            assigned[xy.x][xy.y] = d;
                            all_devices.push_back(d);
                        }
                        delete all_patches;
                    }
                }
            }
        }
    }

    // After we have passed through the whole image, every pixel should be
    // assigned.
    bool parse_succeeded = true;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            if (assigned[x][y] == nullptr) {
                // If the parse failed, save the PNG with the recognized pixels
                // masked out, so the user can see what they did wrong.
                for (size_t x = 0; x < png->get_width(); x++) {
                    for (size_t y = 0; y < png->get_height(); y++) {
                        if (assigned[x][y] != nullptr) {
                            png->set_pixel(x, y, BackgroundDevice::color);
                        }
                    }
                }
                png->write("fail.png");
                return -1;
            }
        }
    }

    // Linking stage: call each device .link() with the assignment map. Each
    // device uses this mapping to figure out 1) what other devices it is
    // connected to, and 2) whether this is a valid usage of the device.
    std::vector<std::tuple<std::string, std::string>> link_fails;
    for (auto i = all_devices.begin(); i != all_devices.end(); i++) {
        std::string fail_string;
        Device *d = *i;
        if (!d->link(assigned, png->get_width(), png->get_height(), &fail_string)) {
            link_fails.push_back(std::make_tuple(d->name(), fail_string));
        }
    }
    if (link_fails.size() > 0) {
        std::cout << "link fails!" << std::endl;
        for (auto i = link_fails.begin(); i != link_fails.end(); i++) {
            std::cout << *i << std::endl;
        }
    }

    // TODO: delete each device in all_devices (delete other stuff?)
    // TODO: move some/all of this into a Parser class

    while (true) {
        // Segment all ports of all devices into nets.
        list<set<Port *>> nets;
        for (auto i = all_devices.begin(); i != all_devices.end(); i++) {
            Device *d = *i;
            for (auto j = d->ports.begin(); j != d->ports.end(); j++) {
                net_segment(&nets, *j);
            }
        }

        // For each net,
        for (auto i = nets.begin(); i != nets.end(); i++) {
            // figure out the total value of the n
            value_t v = VALUE_FLOAT;
            for (auto j = i->begin(); j != i->end(); j++) {
                Port *port = *j;
                if (port->get_value() > v) {
                    v = port->get_value();
                }
            }

            // And apply that value to all ports in the net.
            for (Device *d = net; d != nullptr; d = d->next_in_net) {
                d->set_value(v);
            }
        }
    }

    return 0;
}
