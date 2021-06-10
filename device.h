#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <cstdlib>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "bounding_box.h"
#include "patch.h"
#include "png.h"
#include "port.h"

enum LinkResult {
    CanTouch,
    CanLink,
    LinkError
};

class DeviceOperationException : public std::exception {
public:
    DeviceOperationException(Device *d, std::string s) {
        this->device = d;
        this->message = s;
    }
    Device *device;
    std::string message;
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
    virtual bool parse(AspngSurface *, Coord) = 0;
    Patch flood(AspngSurface *, Coord, Rgb);

    // Return the list of pixels that were parsed out, during the method above.
    virtual std::list<Patch *> all_patches(void) = 0;
    std::set<Coord> all_patches_combined(void);
    BoundingBox get_bounding_box(void);
    Patch *find_patch_containing(Coord);

    // `prelink` indicates whether these devices may touch and whether to
    // create a port. During linking, if `prelink` returns CanLink for both
    // devices, then `link` will be called on both devices, to actually add the
    // ports.
    virtual std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) = 0;
    void add_port(std::shared_ptr<Port>);
    std::list<std::shared_ptr<Port> > all_ports(void);

    // `link` gives each device a chance to finalize the linking process. This
    // is the earliest time that the device knows all ports that are linking
    // into it, and it must resolve "ToBeResolved" port types at this time.
    virtual bool link(void) = 0;

    // `propagate` should return the list of Ports that are immediate neighbors
    // of the given Port.
    virtual std::list<std::shared_ptr<Port> > propagate(std::shared_ptr<Port>) = 0;

    virtual ElectricalValue get_value_at_port(std::shared_ptr<Port>) = 0;

    virtual void apply_new_value(std::shared_ptr<Port>, ElectricalValue) = 0;

    // If the device needs to know when a new simulation step has started,
    // implement this.
    virtual void new_step(void) {};

    virtual void draw(AspngSurface *);
    virtual void draw_debug(AspngSurface *);

    virtual void click(Coord);
    virtual void unclick(void);

protected:
    void draw_helper(AspngSurface *, std::list<Patch *> &);

private:
    std::list<std::shared_ptr<Port> > ports;

    static void flood_helper(Coord, AspngSurface *, Patch &, std::list<Coord> &);
    void maybe_neighbor(Device ***, int32_t, int32_t, int32_t, int32_t, std::set<Device *> *);

    virtual Rgb get_draw_color(Patch *);
};

class BackgroundDevice : public Device {
public:
    BackgroundDevice(void);
    ~BackgroundDevice(void);
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port> > propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(Patch *) override;
};

class BaseTemplateDevice : public Device {
public:
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    static Rgb color;
    void draw(AspngSurface *) override;
    virtual std::string prefix(void) = 0;
    virtual bool sub_parse(AspngSurface *, int32_t, int32_t, int32_t, int32_t, std::string) = 0;
    virtual std::list<Patch *> sub_patches(void) = 0;
    virtual void sub_draw(AspngSurface *, int32_t, int32_t, int32_t, int32_t) = 0;

private:
    Patch patch;
    std::string tab;

    virtual Rgb get_draw_color(Patch *) override;
};

class BridgeDevice : public Device {
public:
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(Patch *) override;
};

class ClockDevice : public BaseTemplateDevice {
public:
    ClockDevice(void);
    std::string name(void) override;
    static Device *create(void);
    std::string prefix(void) override;
    bool sub_parse(AspngSurface *, int32_t, int32_t, int32_t, int32_t, std::string) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    std::list<Patch *> sub_patches(void) override;
    void sub_draw(AspngSurface *, int32_t, int32_t, int32_t, int32_t) override;
    void new_step(void) override;

private:
    ElectricalValue get_state(void);

    uint64_t tick;
    uint32_t divisor;
    Patch sub_patch;
};

class CopperDevice : public Device {
public:
    CopperDevice();
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;

private:
    Patch patch;
    Rgb color_for_drawing;

    virtual Rgb get_draw_color(Patch *) override;
};

class InputDevice : public BaseTemplateDevice {
public:
    InputDevice(void);
    std::string name(void) override;
    static Device *create(void);
    std::string prefix(void) override;
    bool sub_parse(AspngSurface *, int32_t, int32_t, int32_t, int32_t, std::string) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    std::list<Patch *> sub_patches(void) override;
    void sub_draw(AspngSurface *, int32_t, int32_t, int32_t, int32_t) override;
    void click(Coord) override;
    void unclick(void) override;

private:
    bool being_clicked;
    Patch sub_patch;
};

class LEDDevice : public BaseTemplateDevice {
public:
    LEDDevice(void);
    std::string name(void) override;
    static Device *create(void);
    std::string prefix(void) override;
    bool sub_parse(AspngSurface *, int32_t, int32_t, int32_t, int32_t, std::string) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    std::list<Patch *> sub_patches(void) override;
    void sub_draw(AspngSurface *, int32_t, int32_t, int32_t, int32_t) override;

private:
    bool active;
    Patch sub_patch;
};

class PullDevice : public Device {
public:
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb yellow;

private:
    Patch patch_source_or_sink;
    Patch patch_yellow;
    enum {
        PullHi,
        PullLo
    } pull_type;

    virtual Rgb get_draw_color(Patch *) override;
};

class SinkDevice : public Device {
public:
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(Patch *) override;
};

class SourceDevice : public Device {
public:
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;

private:
    Patch patch;

    virtual Rgb get_draw_color(Patch *) override;
};

class SwitchDevice : public BaseTemplateDevice {
public:
    std::string name(void) override;
    static Device *create(void);
    std::string prefix(void) override;
    bool sub_parse(AspngSurface *, int32_t, int32_t, int32_t, int32_t, std::string) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    std::list<Patch *> sub_patches(void) override;
    void sub_draw(AspngSurface *, int32_t, int32_t, int32_t, int32_t) override;
    void click(Coord) override;

private:
    bool closed;
    Patch sub_patch;
};

class TransistorDevice : public Device {
public:
    TransistorDevice();
    std::string name(void) override;
    static Device *create(void);
    bool parse(AspngSurface *, Coord) override;
    std::list<Patch *> all_patches(void) override;
    std::tuple<LinkResult, PortType, std::string> prelink(Patch *, std::shared_ptr<Device>) override;
    bool link(void) override;
    std::list<std::shared_ptr<Port>> propagate(std::shared_ptr<Port>) override;
    ElectricalValue get_value_at_port(std::shared_ptr<Port>) override;
    void apply_new_value(std::shared_ptr<Port>, ElectricalValue) override;
    static Rgb color;
    void draw_debug(AspngSurface *) override;

private:
    Patch patch;
    bool passing;

    virtual Rgb get_draw_color(Patch *) override;
};

#endif
