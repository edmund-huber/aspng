#include "aspng_surface.h"
#include "common.h"

std::ostream &operator<<(std::ostream &os, const Rgb &rgb) {
    std::ios_base::fmtflags f(os.flags());
    os << std::hex;
    os << "Rgb(" << (int)rgb.r << ", " << (int)rgb.g << ", " << (int)rgb.b << ")";
    os.flags(f);
    return os;
}

bool AspngSurface::contains(AspngSurface *other, size_t x, size_t y) {
    if (x + other->get_width() >= this->get_width()) {
        return false;
    }
    if (y + other->get_height() >= this->get_height()) {
        return false;
    }
    for (size_t u = 0; u < other->get_width(); u++) {
        for (size_t v = 0; v < other->get_height(); v++) {
            if (this->get_pixel(x + u, y + v) != other->get_pixel(u, v)) {
                return false;
            }
        }
    }
    return true;
}

void AspngSurface::copy(AspngSurface *other, size_t x, size_t y) {
    ASSERT(x + other->get_width() < this->get_width());
    ASSERT(y + other->get_height() < this->get_height());
    for (size_t u = 0; u < other->get_width(); u++) {
        for (size_t v = 0; v < other->get_height(); v++) {
            this->set_pixel(x + u, y + v, other->get_pixel(u, v));
        }
    }
}
