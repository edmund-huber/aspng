#ifndef __ASPNG_SURFACE_H__
#define __ASPNG_SURFACE_H__

#include <ostream>

#include "patch.h"

class Rgb {
public:
    Rgb() {}
    Rgb(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) {}
    bool operator==(Rgb);
    bool operator!=(Rgb);

    uint8_t r, g, b;
};

std::ostream &operator<<(std::ostream &, const Rgb &);

class AspngSurface {
public:
    virtual Rgb get_pixel(size_t, size_t) = 0;
    virtual void set_pixel(size_t, size_t, Rgb) = 0;
    virtual size_t get_width(void) = 0;
    virtual size_t get_height(void) = 0;
    bool contains(AspngSurface *, size_t, size_t);
    void copy(AspngSurface *, size_t, size_t);
};

#endif
