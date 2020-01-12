#ifndef __PATCH_H__
#define __PATCH_H__

#include <set>
#include <string>

class Coord {
public:
    Coord(void) {};
    Coord(int32_t _x, int32_t _y) : x(_x), y(_y) {};

    int32_t x;
    int32_t y;
};

bool operator<(const Coord, const Coord);
std::string operator+(const std::string, const Coord);

class Patch : private std::set<Coord> {
public:
    using std::set<Coord>::begin;
    using std::set<Coord>::clear;
    using std::set<Coord>::end;
    using std::set<Coord>::find;
    using std::set<Coord>::size;

    void insert(const Coord &);
    void get_bounding_box(int32_t &, int32_t &, int32_t &, int32_t &);
    int32_t get_width(void);
    int32_t get_height(void);
    bool is_subset(Patch &);
};

#endif
