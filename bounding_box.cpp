#include "bounding_box.h"
#include "common.h"

BoundingBox::BoundingBox() {
    this->is_empty = true;
}

Coord BoundingBox::get_bottom_left(void) {
    ASSERT(!this->is_empty);
    return this->bottom_left;
}

Coord BoundingBox::get_top_right(void) {
    ASSERT(!this->is_empty);
    return this->top_right;
}

void BoundingBox::include(Coord coord) {
    if (this->is_empty) {
        this->bottom_left = coord;
        this->top_right = coord;
        this->is_empty = false;
    } else {
        if (coord.x < this->bottom_left.x)
            this->bottom_left.x = coord.x;
        if (coord.x > this->top_right.x)
            this->top_right.x = coord.x;
        if (coord.y < this->bottom_left.y)
            this->bottom_left.y = coord.y;
        if (coord.y > this->top_right.y)
            this->top_right.y = coord.y;
    }
}
