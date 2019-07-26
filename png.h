#ifndef __PNG_H__
#define __PNG_H__

#include <png.h>
#include <string>

class Png {
public:
    static Png *read(std::string);
    void write(std::string);
    size_t get_width();
    size_t get_height();
    void get_pixel(size_t, size_t, uint8_t *);
    void set_pixel(size_t, size_t, uint8_t, uint8_t, uint8_t);

private:
    Png();

    size_t width;
    size_t height;
    png_bytep *rows;
};

#endif
