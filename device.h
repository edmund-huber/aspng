#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <exception>
#include <list>
#include <memory>
#include <set>

#include "patch.h"
#include "png.h"

enum ElectricalValue {
    EmptyElectricalValue,
    HiElectricalValue,
    LoElectricalValue
};

ElectricalValue combine_electrical_values(ElectricalValue, ElectricalValue);
std::string electrical_value_to_str(ElectricalValue);

class ElectricalValueException : public std::exception {
    virtual const char *what() const throw() {
        return "ElectricalValueException";
    }
};

enum LinkResult {
    CanTouch,
    CanLink,
    LinkError
};

// At link time, a device determines what the port means to itself, so that
// later, at simulation time, it knows what to do with a port.
enum PortType {
    NoSpecialMeaning
};

class Device;

class Port {
public:
    Port(std::shared_ptr<Device> _d1, PortType _d1_port_type, std::shared_ptr<Device> _d2, PortType _d2_port_type) : d1(_d1), d1_port_type(_d1_port_type), d2(_d2), d2_port_type(_d2_port_type) {};
    PortType get_port_type(std::shared_ptr<Device> d1);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue compute_new_value(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);

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
    virtual std::tuple<LinkResult, PortType> prelink(std::shared_ptr<Device>) = 0;
    void add_port(std::shared_ptr<Port>);
    std::list<std::shared_ptr<Port>> all_ports(void);

    // `propagate` should return the list of Ports that are immediate neighbors
    // of the given Port.
    virtual std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) = 0;

    virtual ElectricalValue get_value_at_port(std::shared_ptr<Port>) = 0;

    virtual void apply_new_value(ElectricalValue) = 0;

    void draw(Png *);

private:
    std::list<std::shared_ptr<Port>> ports;

    static void flood_helper(Png *, size_t, size_t, Rgb, Patch &, Patch &);
    void maybe_neighbor(Device ***, size_t, size_t, size_t, size_t, std::set<Device *> *);

    virtual Rgb get_draw_color(void) = 0;
};

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    std::tuple<LinkResult, PortType> prelink(std::shared_ptr<Device>);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue get_value_at_port(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(void);
};

class CopperDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    std::tuple<LinkResult, PortType> prelink(std::shared_ptr<Device>);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue get_value_at_port(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);
    static Rgb color;

private:
    Patch patch;
    Rgb color_for_drawing;

    virtual Rgb get_draw_color(void);
};

class SinkDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    std::tuple<LinkResult, PortType> prelink(std::shared_ptr<Device>);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue get_value_at_port(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(void);
};

class SourceDevice : public Device {
public:
    std::string name(void);
    static Device *create(void);
    bool parse(Png *, size_t, size_t);
    std::list<Patch> all_patches(void);
    std::tuple<LinkResult, PortType> prelink(std::shared_ptr<Device>);
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>);
    ElectricalValue get_value_at_port(std::shared_ptr<Port>);
    void apply_new_value(ElectricalValue);
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(void);
};

#endif
