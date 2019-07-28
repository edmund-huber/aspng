#include <functional>
#include <vector>

#include "common.h"
#include "png.h"

class Device {
public:
    virtual bool take(int, int, Rgb &) = 0;
};

class SourceDevice : public Device {
public:
    static Device *create(void);
    bool take(int, int, Rgb &);
};

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::take(int x, int y, Rgb &rgb) {
    return rgb.equals(Rgb(0xff, 0xff, 0xff));
}

class SinkDevice : public Device {
public:
    static Device *create(void);
    bool take(int, int, Rgb &);
};

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

bool SinkDevice::take(int x, int y, Rgb &rgb) {
    return rgb.equals(Rgb(0, 0, 0));
}

void expand(Png *png, size_t x, size_t y, Device *d, bool **assigned) {
    if ((x < 0) || (x >= png->get_width()) || (y < 0) || (y >= png->get_height())) {
        return;
    }
    if (assigned[x][y]) {
        return;
    }
    Rgb rgb = png->get_pixel(x, y);
    if (d->take(x, y, rgb)) {
        assigned[x][y] = true;
        expand(png, x + 1, y, d, assigned);
        expand(png, x, y + 1, d, assigned);
        expand(png, x - 1, y, d, assigned);
        expand(png, x, y - 1, d, assigned);
    }
}

int main(void) {
    auto png = Png::read("tests/basic_source_sink/_.png");

    bool **assigned = new bool*[png->get_width()];
    for (size_t x = 0; x < png->get_width(); x++) {
        assigned[x] = new bool[png->get_height()];
        for (size_t y = 0; y < png->get_height(); y++) {
            assigned[x][y] = false;
        }
    }

    std::vector<std::function<Device *(void)>> registry;
    registry.push_back(SinkDevice::create);
    registry.push_back(SourceDevice::create);

    // Go through all pixels until we find an unassigned pixel, and try to
    // start a device there.
    for (size_t y = 0; y < png->get_height(); y++) {
        for (size_t x = 0; x < png->get_width(); x++) {
            if (!assigned[x][y * png->get_width()]) {
                for (auto di = registry.begin(); di != registry.end(); di++) {
                    Device *d = (*di)();
                    // Expand the device in all directions.
                    expand(png, x, y, d, assigned);
                }
            }
        }
    }

    // After we have passed through the whole image, every pixel should be
    // assigned, because everything got parsed out.
    bool parse_succeeded = true;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            if (!assigned[x][y]) {
                parse_succeeded = false;
                break;
            }
        }
    }

    // If the parse failed, save the PNG with the recognized pixels masked out,
    // so the user can see what they did wrong.
    if (!parse_succeeded) {
        for (size_t x = 0; x < png->get_width(); x++) {
            for (size_t y = 0; y < png->get_height(); y++) {
                if (assigned[x][y]) {
                    png->set_pixel(x, y, Rgb(0x99, 0x99, 0x99));
                }
            }
        }
        png->write("fail.png");
        return -1;
    }

    return 0;
}
