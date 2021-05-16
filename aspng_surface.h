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
    virtual ~AspngSurface() {}
    virtual Rgb get_pixel(int32_t, int32_t) = 0;
    virtual void set_pixel(int32_t, int32_t, Rgb) = 0;
    virtual int32_t get_width(void) = 0;
    virtual int32_t get_height(void) = 0;
    virtual void start_draw(void);
    virtual void finish_draw(void);
    bool contains(AspngSurface *, int32_t, int32_t);
    void copy(AspngSurface *, int32_t, int32_t);
};

#endif
