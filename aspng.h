#ifndef __ASPNG_H__
#define __ASPNG_H__

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "aspng_surface.h"
#include "bounding_box.h"
#include "device.h"
#include "net.h"
#include "patch.h"

class AspngSimException : public std::exception {
public:
    AspngSimException(std::string _message, BoundingBox _bounding_box) : message(_message), bounding_box(_bounding_box) {}
    AspngSimException() {}

    std::string message;
    BoundingBox bounding_box;
};

class Aspng {
public:
    Aspng(AspngSurface *, std::string &);
    void step(void);
    void draw(AspngSurface *);
    void draw_input_map(AspngSurface *);
    std::shared_ptr<Device> which_device(Coord);

private:
    std::set<std::shared_ptr<Device>> all_devices;
    std::set<std::shared_ptr<Port>> all_ports;

    std::string maybe_add_ports(std::map<Coord, std::shared_ptr<Device>> &, Coord, int32_t, int32_t);

    bool first_step;
    std::set<Net> previous_nets;
};

#endif
