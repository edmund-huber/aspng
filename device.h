#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <set>
#include <vector>

#include "png.h"

typedef enum {
    VALUE_FLOAT = 0,
    VALUE_HIGH,
    VALUE_LOW
} value_t;

class Coord2d {
public:
    Coord2d(size_t x, size_t y) : x(x), y(y) {}
    bool operator==(Coord2d);
    size_t x, y;
};

typedef std::vector<Coord2d> Patch;

class Device {
public:
    virtual ~Device(void) {};

    virtual std::string name(void) = 0;

    // Given an image, can a device of this type be parsed, starting from a
    // given coordinate? (The device is responsible for remembering which
    // pixels it parsed out.) Note that this is separate from "linking" -- we
    // do not decide here whether the device can exist in the context of the
    // pixels (and devices) surrounding it.
    virtual bool parse(Png *, size_t, size_t) = 0;
    Patch *parse_flood(Png *, size_t, size_t, Rgb);

    // Return the list of pixels that were parsed out, during the method above.
    // The caller will delete the list.
    virtual Patch *all_patches(void) = 0;

    // Given a 2D array of Device *, figure out (generally, based on the
    // neighbors) whether this device is being used in a valid way. If the link
    // failed, fail_str should explain what went wrong.
    virtual bool link(Device ***, size_t, size_t, std::string *) = 0;
    void link_find_neighbors(Device ***, size_t, size_t, std::set<Device *> *);

    // Given the device's current state, which ports
    virtual void propagate(Port *, list<Port *> *);

private:
    static void flood_helper(Png *, size_t, size_t, Rgb, Patch *, Patch *); // TODO probably more convenient if not static
    void maybe_neighbor(Device ***, size_t, size_t, size_t, size_t, std::set<Device *> *);
};

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    bool link(Device ***, size_t, size_t, std::string *);
    static Rgb color;

private:
    Patch *patch;
};

class CopperDevice : public Device {
public:
    CopperDevice(void);
    ~CopperDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    bool link(Device ***, size_t, size_t, std::string *);
    static Rgb color;

private:
    Patch *patch;
    std::set<Device *> neighbors;
};

class SinkDevice : public Device {
public:
    SinkDevice(void);
    ~SinkDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    bool link(Device ***, size_t, size_t, std::string *);
    static Rgb color;

private:
    Patch *patch;
    std::set<Device *> neighbors;
};

class SourceDevice : public Device {
public:
    SourceDevice(void);
    ~SourceDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    Patch *all_patches(void);
    bool link(Device ***, size_t, size_t, std::string *);
    static Rgb color;

private:
    Patch *patch;
    std::set<Device *> neighbors;
};

#endif
