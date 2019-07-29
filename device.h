#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <vector>

#include "png.h"

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

    // Helpers for parsing.
    Patch *flood(Png *, size_t, size_t, Rgb);

private:
    static void _flood(Png *, size_t, size_t, Rgb, Patch *, Patch *);
};

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

#endif
