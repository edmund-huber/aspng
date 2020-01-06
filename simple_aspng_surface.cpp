#include <cstring>

#include "common.h"
#include "simple_aspng_surface.h"

SimpleAspngSurface::SimpleAspngSurface(size_t w, size_t h) {
    this->width = w;
    this->height = h;
    this->pixels.resize(w * h);
}

void SimpleAspngSurface::resize(size_t w, size_t h) {
    std::vector<Rgb> new_pixels(w * h);
    for (size_t x = 0; x < this->width; x++) {
        for (size_t y = 0; y < this->height; y++) {
            if ((x < w) && (y < h)) {
                new_pixels[(w * y) + x] = this->pixels[(this->width * y) + x];
            }
        }
    }
    this->pixels = new_pixels;
    this->width = w;
    this->height = h;
}

size_t SimpleAspngSurface::get_width(void) {
    return this->width;
}

size_t SimpleAspngSurface::get_height(void) {
    return this->height;
}

Rgb SimpleAspngSurface::get_pixel(size_t x, size_t y) {
    ASSERT((x >= 0) && (x < this->width));
    ASSERT((y >= 0) && (y < this->height));
    return this->pixels[(this->width * y) + x];
}

void SimpleAspngSurface::set_pixel(size_t x, size_t y, Rgb rgb) {
    ASSERT((x >= 0) && (x < this->width));
    ASSERT((y >= 0) && (y < this->height));
    this->pixels[(this->width * y) + x] = rgb;
}
