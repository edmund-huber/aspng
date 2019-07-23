#ifndef __PNG_H__
#define __PNG_H__

#include <png.h>
#include <string>

class Png {
public:
    static Png *open(std::string);
    int width;
    int height;
    png_bytep *rows;

private:
    Png();
};

#endif
