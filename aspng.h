#ifndef __ASPNG_H__
#define __ASPNG_H__

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "aspng_surface.h"
#include "device.h"
#include "patch.h"

class Aspng {
public:
    Aspng(AspngSurface *, std::string &);
    std::string step(void);
    void draw(AspngSurface *);
    void draw_input_map(AspngSurface *);
    std::shared_ptr<Device> which_device(Coord);

private:
    std::set<std::shared_ptr<Device>> all_devices;
    std::set<std::shared_ptr<Port>> all_ports;

    bool maybe_add_ports(std::map<Coord, std::shared_ptr<Device>> &, Coord, size_t, size_t);
};

#endif
