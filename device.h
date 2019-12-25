#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <list>
#include <memory>
#include <set>

#include "patch.h"
#include "png.h"

enum LinkResult {
    CanTouch,
    CanLink,
    LinkError
};

// At link time, a device determines what the port means to itself, so that
// later, at simulation time, it knows what to do with a port.
enum PortType {
    NotAPort, // Only for use by simulator.
};

class Device;

class Port {
public:
    Port(std::shared_ptr<Device> _d1, PortType _d1_port_type, std::shared_ptr<Device> _d2, PortType _d2_port_type) : d1(_d1), d1_port_type(_d1_port_type), d2(_d2), d2_port_type(_d2_port_type) {};

    PortType get_port_type(std::shared_ptr<Device> d1);

private:
    std::shared_ptr<Device> d1;
    PortType d1_port_type;
    std::shared_ptr<Device> d2;
    PortType d2_port_type;
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

    // `prelink` indicates whether these devices may touch and whether to
    // create a port. During linking, if `prelink` returns CanLink for both
    // devices, then `link` will be called on both devices, to actually add the
    // ports.
    virtual LinkResult prelink(std::shared_ptr<Device>, PortType &) = 0;
    void add_port(std::shared_ptr<Port>);

private:
    std::list<std::shared_ptr<Port>> ports;

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
    LinkResult prelink(std::shared_ptr<Device>);
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
    LinkResult prelink(std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
};

class SinkDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    LinkResult prelink(std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
};

class SourceDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    LinkResult prelink(std::shared_ptr<Device>);
    static Rgb color;

private:
    Patch patch;
};

#endif
