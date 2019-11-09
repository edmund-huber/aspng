#include <algorithm>
#include <assert.h>

#include "patch.h"

void Patch::push_back(const Coord &coord) {
    std::vector<Coord>::push_back(coord);

    // Invariant: patch must be contiguous. Unfortunately, I can check here,
    // but I can't stop the user of the iterator from changing Coord values.
    bool is_contiguous = false;
    if (this->size() == 1) {
        is_contiguous = true;
    } else {
        is_contiguous =
            (std::find(this->begin(), this->end(), Coord(coord.x + 1, coord.y)) != this->end())
            || (std::find(this->begin(), this->end(), Coord(coord.x - 1, coord.y)) != this->end())
            || (std::find(this->begin(), this->end(), Coord(coord.x, coord.y + 1)) != this->end())
            || (std::find(this->begin(), this->end(), Coord(coord.x, coord.y - 1)) != this->end());
    }
    assert(is_contiguous);
}
