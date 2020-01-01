#ifndef __PNG_H__
#define __PNG_H__

#include <png.h>
#include <string>

#include "aspngsurface.h"

class Png : public AspngSurface {
public:
    Png(size_t, size_t);
    static Png *read(std::string);
    void write(std::string);
    size_t get_width();
    size_t get_height();
    Rgb get_pixel(size_t, size_t);
    void set_pixel(size_t, size_t, Rgb);

private:
    Png();

    size_t width;
    size_t height;
    png_bytep *rows;
};

#endif
