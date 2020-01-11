#ifndef __PNG_H__
#define __PNG_H__

#include <png.h>
#include <string>

#include "aspng_surface.h"

class Png : public AspngSurface {
public:
    Png(int32_t, int32_t);
    static Png *read(std::string);
    void write(std::string);
    int32_t get_width();
    int32_t get_height();
    Rgb get_pixel(int32_t, int32_t);
    void set_pixel(int32_t, int32_t, Rgb);

private:
    Png();

    int32_t width;
    int32_t height;
    png_bytep *rows;
};

#endif
