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
    virtual bool parse(Png *, size_t, size_t) = 0;
    virtual Patch *all_patches(void) = 0;
    virtual bool link(Device ***, size_t, size_t, std::string *) = 0;

    // Helpers for parsing & linking.
    Patch *flood(Png *, size_t, size_t, Rgb);
    void find_neighbors(Device ***, size_t, size_t, std::set<Device *> *);

    Device *head_of_net;
    Device *next_in_net;
    Device *next_head_of_net;

private:
    static void flood_helper(Png *, size_t, size_t, Rgb, Patch *, Patch *); // TODO probably more convenient if not static
    void maybe_neighbor(Device ***, size_t, size_t, size_t, size_t, std::set<Device *> *);
};

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
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
