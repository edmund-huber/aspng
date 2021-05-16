#ifndef __COORD_H__
#define __COORD_H__

#include <stdint.h>
#include <string>

class Coord {
public:
    Coord(void) {}
    Coord(int32_t _x, int32_t _y) : x(_x), y(_y) {}

    int32_t x;
    int32_t y;
};

bool operator<(const Coord, const Coord);
std::string operator+(const std::string, const Coord);

#endif
