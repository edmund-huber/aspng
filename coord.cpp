#include "coord.h"

bool Coord::operator==(Coord other) {
    return (this->x == other.x) && (this->y == other.y);
}
