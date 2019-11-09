#ifndef __COORD_H__
#define __COORD_H__

#include <stddef.h>

class Coord {
public:
    Coord(size_t x, size_t y) : x(x), y(y) {}
    bool operator==(Coord);
    size_t x, y;
};

#endif
