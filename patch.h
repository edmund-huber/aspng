#ifndef __PATCH_H__
#define __PATCH_H__

#include <set>
#include <tuple>

typedef std::tuple<size_t, size_t> Coord;

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
};

#endif
