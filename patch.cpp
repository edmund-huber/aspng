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

void Patch::get_bounding_box(int32_t &min_x, int32_t &max_x, int32_t &min_y, int32_t &max_y) {
    min_x = INT32_MAX;
    max_x = INT32_MIN;
    min_y = INT32_MAX;
    max_y = INT32_MIN;
    for (auto i = this->begin(); i != this->end(); i++) {
        auto coord = *i;
        int32_t x, y;
        std::tie(x, y) = coord;
        min_x = x < min_x ? x : min_x;
        max_x = x > max_x ? x : max_x;
        min_y = y < min_y ? y : min_y;
        max_y = y > max_y ? y : max_y;
    }
}

int32_t Patch::get_width(void) {
    int32_t min_x, max_x, min_y, max_y;
    this->get_bounding_box(min_x, max_x, min_y, max_y);
    return max_x - min_x;
}

int32_t Patch::get_height(void) {
    int32_t min_x, max_x, min_y, max_y;
    this->get_bounding_box(min_x, max_x, min_y, max_y);
    return max_y - min_y;
}
