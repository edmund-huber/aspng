#include <iostream>
#include <functional>
#include <vector>

#include "common.h"
#include "device.h"
#include "png.h"

/*this->patch1 = this->flood(init_x, init_y, ViaDevice::color);
    if (this->patch1 == nullptr) {
        return false;
    }
    BoundingBox bbox1 = this->patch1->bounding_box();
    if ((bbox1.x1 - bbox1.x0 == 0) && (bbox1.y1 - bbox1.y0 > 0)) {
        // Since it's a vertical via, when we peek right, we should find
        // only copper, ..
        for (size_t u = 1; ; u++) {
            for (size_t v = 0; ; v++) {
                if (!this->peek(bbox1.x0 + u, bbox1.y0 + v, &rgb)) {
                    return false;
                } else if (Copper::all_colors.contains(rgb)) {
                    continue;
                } else if (rgb == ViaDevice::color) {
                    // .. until we find the other via.
                    break;
                } else {
                    return false;
                }
            }
        }
        // The 2nd via must mirror the 1st via.
        this->patch2 = this.flood(bbox1.x0 + u, bbox1.y0, ViaDevice::color);
        ASSERT(patch2 != nullptr);
        BoundingBox bbox2 = this->patch2->bounding_box();
        // TODO: check patch2.size() == y1 - y0
        if ((bbox2.x1 - bbox2.x0 == 0) && (bbox2.y0 == bbox1.y0) && (bbox2.y1 == bbox1.y1)) {
            return true;
        } else {
            return false;
        }
    } else if (....) {
        // ... TODO horizontal case ....
    } else {
        return false;
    }
}*/

void net_segment(Device *root, Device *d) {
    // If this Device is already in this net, we can stop.
    if (d->head_of_net == root) return;

    // If this Device is in another net, then something has gone horribly
    // wrong.
    ASSERT(d->head_of_net == nullptr)

    // This device is in the current net.
    d->head_of_net = root;
    d->next_in_net = root->next_in_net;
    root->next_in_net = d;

    // And all the Devices connected to it are, too.
    auto connected = d->connected_devices();
    for (auto iter = connected.begin(); iter != connected.end(); iter++) {
        net_segment(root, *iter);
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
                parse_succeeded = false;
                break;
            }
        }
    }

    // Linking stage: call each device .link() with the assignment map. Each
    // device uses this mapping to figure out 1) what other devices it is
    // connected to, and 2) whether this is a valid usage of the device.
    std::vector<std::string> link_fails;
    for (auto i = all_devices.begin(); i != all_devices.end(); i++) {
        std::string fail_string;
        if (!(*i)->link(assigned, png->get_width(), png->get_height(), &fail_string)) {
            link_fails.push_back(fail_string);
        }
    }
    if (link_fails.size() > 0) {
        std::cout << "link fails!" << std::endl;
        for (auto i = link_fails.begin(); i != link_fails.end(); i++) {
            std::cout << *i << std::endl;
        }
    }

    // TODO: how to connect to other devices? -> after parsing of whole image succeeds,
    //                                then "linking" stage - Device::link called with pixel->device mapping
    //                                (.. for each device)

    // If the parse failed, save the PNG with the recognized pixels masked out,
    // so the user can see what they did wrong.
    if (!parse_succeeded) {
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

    // TODO: delete each device in all_devices (delete other stuff?)
    // TODO: move some/all of this into a Parser class

    while (true) {
        // Segment all devices into nets.
        for (auto iter = all_devices.begin(); iter != all_devices.end(); iter++) {
            (*iter)->head_of_net = nullptr;
            (*iter)->next_in_net = nullptr;
            (*iter)->next_head_of_net = nullptr;
        }
        Device *first_head_of_net = nullptr;
        for (auto iter = all_devices.begin(); iter != all_devices.end(); iter++) {
            net_segment(*iter, *iter);
            (*iter)->next_head_of_net = first_head_of_net;
            first_head_of_net = *iter;
        }

        // For each net,
        for (Device *net = first_head_of_net; net != nullptr; net = net->next_head_of_net) {
            // figure out the total value of the net.
            value_t v = VALUE_FLOAT;
            for (Device *d = net; d != nullptr; d = d->next_in_net) {
                if (d->get_value() > v) {
                    v = d->get_value();
                }
            }

            // And apply that value to all devices in the net.
            for (Device *d = net; d != nullptr; d = d->next_in_net) {
                d->set_value(v);
            }
        }
    }

    return 0;
}
