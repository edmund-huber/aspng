#ifndef __SIMPLE_ASPNG_SURFACE_H__
#define __SIMPLE_ASPNG_SURFACE_H__

#include <vector>

#include "aspng.h"

class SimpleAspngSurface : public AspngSurface {
public:
    SimpleAspngSurface(int32_t, int32_t);
    void resize(int32_t, int32_t);
    Rgb get_pixel(int32_t, int32_t);
    void set_pixel(int32_t, int32_t, Rgb);
    int32_t get_width(void);
    int32_t get_height(void);

private:
    int32_t width;
    int32_t height;
    std::vector<Rgb> pixels;
};

#endif
