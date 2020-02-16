#ifndef __BOUNDING_BOX_H__
#define __BOUNDING_BOX_H__

#include "coord.h"

class BoundingBox {
public:
    BoundingBox(void);
    Coord get_bottom_left(void);
    Coord get_top_right(void);
    void include(Coord);

private:
    bool is_empty;
    Coord bottom_left;
    Coord top_right;
};

#endif
