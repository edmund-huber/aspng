#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <list>
#include <memory>
#include <set>

#include "patch.h"
#include "png.h"

enum PortType {
    InvalidPort,
    BackgroundPort,
    CopperPort,
    SourcePort,
    SinkPort
};

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
    Patch flood(Png *, size_t, size_t, Rgb);

    // Return the list of pixels that were parsed out, during the method above.
    virtual std::list<Patch> all_patches(void) = 0;

    // Can this device, through the given patch (which is part of this device),
    // link to this otherother device? If yes, the patch is a port, and return
    // a value indicating what this port means to this device. Or return
    // InvalidPort.
    virtual PortType link(std::shared_ptr<Patch>, std::shared_ptr<Device>) = 0;

private:
    static void flood_helper(Png *, size_t, size_t, Rgb, Patch &, Patch &);
    void maybe_neighbor(Device ***, size_t, size_t, size_t, size_t, std::set<Device *> *);
};

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    PortType link(std::shared_ptr<Patch>, std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
};

class CopperDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    PortType link(std::shared_ptr<Patch>, std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
    std::set<Device *> neighbors;
};

class SinkDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    PortType link(std::shared_ptr<Patch>, std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
    std::set<Device *> neighbors;
};

class SourceDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    PortType link(std::shared_ptr<Patch>, std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
    std::set<Device *> neighbors;
};

class Port {
private:
    std::shared_ptr<Device> other_device;
    Patch patch;
};

#endif
