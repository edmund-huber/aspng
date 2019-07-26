#include <iostream>
#include <functional>
#include <vector>

#include "common.h"
#include "png.h"

class Device {
public:
    virtual bool take(int, int, uint8_t*) = 0;
};

class SourceDevice : public Device {
public:
    static Device *create(void);
    bool take(int, int, uint8_t*);
};

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::take(int x, int y, uint8_t *rgb) {
    return false;
}

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    auto assigned = new bool*[png->get_width()];
    for (size_t x = 0; x < png->get_width(); x++) {
        assigned[x] = new bool[png->get_height()];
        for (size_t y = 0; y < png->get_height(); y++) {
            assigned[x][y] = false;
        }
    }

    std::vector<std::function<Device *(void)>> registry;
    registry.push_back(SourceDevice::create);

    size_t old_assigned_count = 0;
    while (true) {
        // Count up all the assigned pixels.

        // Go through all pixels until we find an unassigned pixel, and try to
        // start a device there.
        for (size_t y = 0; y < png->get_height(); y++) {
            for (size_t x = 0; x < png->get_width(); x++) {
                if (!assigned[x][y * png->get_width()]) {
                    for (auto di = registry.begin(); di != registry.end(); di++) {
                        Device *d = (*di)();

                        // Expand the device in all directions.

                        uint8_t rgb[3];
                        png->get_pixel(x, y, rgb);
                        d->take(x, y, rgb);
                    }
                    //}
                }
            }
        }

        // We're done if all the pixels are assigned.
        size_t assigned_count = 0;
        for (size_t x = 0; x < png->get_width(); x++) {
            for (size_t y = 0; y < png->get_height(); y++) {
                if (assigned[x][y]) {
                    assigned_count++;
                }
            }
        }
        if (assigned_count == png->get_width() * png->get_height()) {
            break;
        }

        // (If there hasn't been any progress, then we can't parse this image.)
        if (assigned_count == old_assigned_count) {
            // Write out the design with the recognized pixels masked out.
            for (size_t x = 0; x < png->get_width(); x++) {
                for (size_t y = 0; y < png->get_height(); y++) {
                    if (!assigned[x][y]) {
                        png->set_pixel(x, y, 0, 0, 0);
                    }
                }
            }
            png->write("fail.png");
            return -1;
        }
        old_assigned_count = assigned_count;
    }

    return 0;
}
