#ifndef __PATCH_H__
#define __PATCH_H__

#include <vector>
#include "coord.h"

class Patch : private std::vector<Coord> {
public:
    using std::vector<Coord>::begin;
    using std::vector<Coord>::end;
    using std::vector<Coord>::size;

    void push_back(const Coord &);
};

#endif
