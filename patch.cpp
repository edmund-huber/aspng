#include "common.h"
#include "patch.h"

bool operator<(const Coord c1, const Coord c2) {
    if (c1.x != c2.x) return c1.x < c2.x;
    return c1.y < c2.y;
}

std::string operator+(const std::string s, const Coord c) {
    return s + "(" + std::to_string(c.x) + ", " + std::to_string(c.y) + ")";
}

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
            (this->find(Coord(coord.x + 1, coord.y)) != this->end())
            || (this->find(Coord(coord.x - 1, coord.y)) != this->end())
            || (this->find(Coord(coord.x, coord.y + 1)) != this->end())
            || (this->find(Coord(coord.x, coord.y - 1)) != this->end());
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
        min_x = coord.x < min_x ? coord.x : min_x;
        max_x = coord.x > max_x ? coord.x : max_x;
        min_y = coord.y < min_y ? coord.y : min_y;
        max_y = coord.y > max_y ? coord.y : max_y;
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

bool Patch::is_subset(Patch &other) {
    for (auto i = this->begin(); i != this->end(); i++) {
        if (other.find(*i) == other.end()) {
            return false;
        }
    }
    return true;
}
