#include "coord.h"

bool operator<(const Coord c1, const Coord c2) {
    if (c1.x != c2.x) return c1.x < c2.x;
    return c1.y < c2.y;
}

std::string operator+(const std::string s, const Coord c) {
    return s + "(" + std::to_string(c.x) + ", " + std::to_string(c.y) + ")";
}
