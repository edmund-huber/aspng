#include <algorithm>
#include <functional>
#include <vector>

#include "common.h"
#include "png.h"

class Coord2d {
public:
    Coord2d(size_t x, size_t y) : x(x), y(y) {}
    bool operator==(Coord2d);
    size_t x, y;
};

bool Coord2d::operator==(Coord2d other) {
    return (this->x == other.x) && (this->y == other.y);
}

typedef std::vector<Coord2d> Patch;

class Device {
public:
    virtual ~Device(void) {};
    virtual bool parse(Png *, size_t, size_t) = 0;
    virtual Patch *all_patches(void) = 0;

    // Helpers for parsing.
    Patch *flood(Png *, size_t, size_t, Rgb);

private:
    static void _flood(Png *, size_t, size_t, Rgb, Patch *, Patch *);
};

Patch *Device::flood(Png *png, size_t x, size_t y, Rgb color) {
    Patch *patch = new Patch();
    Patch visited;
    Device::_flood(png, x, y, color, patch, &visited);
    return patch;
}

void Device::_flood(Png *png, size_t x, size_t y, Rgb color, Patch *patch, Patch *visited) {
    if ((x < 0) || (x >= png->get_width())) {
        return;
    }
    if ((y < 0) || (y >= png->get_height())) {
        return;
    }
    if (std::find(visited->begin(), visited->end(), Coord2d(x, y)) != visited->end()) {
        return;
    }
    visited->push_back(Coord2d(x, y));
    if (png->get_pixel(x, y) == color) {
        patch->push_back(Coord2d(x, y));
        Device::_flood(png, x - 1, y, color, patch, visited);
        Device::_flood(png, x + 1, y, color, patch, visited);
        Device::_flood(png, x, y - 1, color, patch, visited);
        Device::_flood(png, x, y + 1, color, patch, visited);
    }
}

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    static Rgb color;

private:
    Patch *patch;
};

BackgroundDevice::BackgroundDevice(void) {
    this->patch = nullptr;
}

BackgroundDevice::~BackgroundDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Rgb BackgroundDevice::color = Rgb(0x99, 0x99, 0x99);

Device *BackgroundDevice::create(void) {
    return new BackgroundDevice();
}

bool BackgroundDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, BackgroundDevice::color);
    return this->patch->size() >= 1;
}

Patch *BackgroundDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

class SourceDevice : public Device {
public:
    SourceDevice(void);
    ~SourceDevice(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    static Rgb color;

private:
    Patch *patch;
};

SourceDevice::SourceDevice(void) {
    this->patch = nullptr;
}

SourceDevice::~SourceDevice(void) {
    if (this->patch != nullptr) {
       delete this->patch;
    }
}

Rgb SourceDevice::color = Rgb(0xff, 0xff, 0xff);

Device *SourceDevice::create(void) {
    return new SourceDevice();
}

bool SourceDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SourceDevice::color);
    return this->patch->size() == 1;
}

Patch *SourceDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

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

class SinkDevice : public Device {
public:
    SinkDevice(void);
    ~SinkDevice(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    static Rgb color;

private:
    Patch *patch;
};

SinkDevice::SinkDevice(void) {
    this->patch = nullptr;
}

SinkDevice::~SinkDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Device *SinkDevice::create(void) {
    return new SinkDevice();
}

Rgb SinkDevice::color = Rgb(0, 0, 0);

bool SinkDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, SinkDevice::color);
    return this->patch->size() == 1;
}

Patch *SinkDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
}

class CopperDevice : public Device {
public:
    CopperDevice(void);
    ~CopperDevice(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    static Rgb color;

private:
    Patch *patch;
};

CopperDevice::CopperDevice(void) {
    this->patch = nullptr;
}

CopperDevice::~CopperDevice(void) {
    if (this->patch != nullptr) {
        delete this->patch;
    }
}

Device *CopperDevice::create(void) {
    return new CopperDevice();
}

Rgb CopperDevice::color = Rgb(0xdb, 0x73, 0);

bool CopperDevice::parse(Png *png, size_t x, size_t y) {
    this->patch = this->flood(png, x, y, CopperDevice::color);
    return this->patch->size() == 1;
}

Patch *CopperDevice::all_patches(void) {
    Patch *all_patches = new Patch();
    all_patches->insert(all_patches->end(), this->patch->begin(), this->patch->end());
    return all_patches;
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
                        }
                        delete all_patches;
                    }
                }
            }
        }
    }

    // After we have passed through the whole image, every pixel should be
    // assigned, because everything got parsed out.
    bool parse_succeeded = true;
    for (size_t x = 0; x < png->get_width(); x++) {
        for (size_t y = 0; y < png->get_height(); y++) {
            if (assigned[x][y] == nullptr) {
                parse_succeeded = false;
                break;
            }
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

    return 0;
}
