#ifndef __SIMPLE_ASPNG_SURFACE_H__
#define __SIMPLE_ASPNG_SURFACE_H__

#include <vector>

#include "aspng.h"

class SimpleAspngSurface : public AspngSurface {
public:
    SimpleAspngSurface(size_t, size_t);
    void resize(size_t, size_t);
    Rgb get_pixel(size_t, size_t);
    void set_pixel(size_t, size_t, Rgb);
    size_t get_width(void);
    size_t get_height(void);

private:
    size_t width;
    size_t height;
    std::vector<Rgb> pixels;
};

#endif
