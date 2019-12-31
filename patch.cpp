#include "common.h"
#include "patch.h"

void Patch::insert(const Coord &coord) {
    std::set<Coord>::insert(coord);

    // Invariant: patch must be contiguous. Unfortunately, I can check here,
    // but I can't e.g. stop the user of the iterator from changing Coord
    // values.
    bool is_contiguous = false;
    if (this->size() == 1) {
        is_contiguous = true;
    } else {
        is_contiguous =
            (this->find(Coord(std::get<0>(coord) + 1, std::get<1>(coord))) != this->end())
            || (this->find(Coord(std::get<0>(coord) - 1, std::get<1>(coord))) != this->end())
            || (this->find(Coord(std::get<0>(coord), std::get<1>(coord) + 1)) != this->end())
            || (this->find(Coord(std::get<0>(coord), std::get<1>(coord) - 1)) != this->end());
    }
    ASSERT(is_contiguous);
}
