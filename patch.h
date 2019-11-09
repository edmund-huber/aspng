#ifndef __PATCH_H__
#define __PATCH_H__

#include <vector>
#include "coord.h"

class Patch : private std::vector<Coord> {
public:
    using std::vector<Coord>::begin;
    using std::vector<Coord>::end;
    using std::vector<Coord>::push_back;
    using std::vector<Coord>::size;
};

#endif
