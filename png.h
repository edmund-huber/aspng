#ifndef __PNG_H__
#define __PNG_H__

#include <png.h>
#include <string>

class Rgb {
public:
    Rgb(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) {}
    bool equals(Rgb);

    uint8_t r, g, b;
};

class Png {
public:
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
