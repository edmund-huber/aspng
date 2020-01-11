#include <cstring>

#include "common.h"
#include "simple_aspng_surface.h"

SimpleAspngSurface::SimpleAspngSurface(int32_t w, int32_t h) {
    this->width = w;
    this->height = h;
    this->pixels.resize(w * h);
}

void SimpleAspngSurface::resize(int32_t w, int32_t h) {
    std::vector<Rgb> new_pixels(w * h);
    for (int32_t x = 0; x < this->width; x++) {
        for (int32_t y = 0; y < this->height; y++) {
            if ((x < w) && (y < h)) {
                new_pixels[(w * y) + x] = this->pixels[(this->width * y) + x];
            }
        }
    }
    this->pixels = new_pixels;
    this->width = w;
    this->height = h;
}

int32_t SimpleAspngSurface::get_width(void) {
    return this->width;
}

int32_t SimpleAspngSurface::get_height(void) {
    return this->height;
}

Rgb SimpleAspngSurface::get_pixel(int32_t x, int32_t y) {
    ASSERT((x >= 0) && (x < this->width));
    ASSERT((y >= 0) && (y < this->height));
    return this->pixels[(this->width * y) + x];
}

void SimpleAspngSurface::set_pixel(int32_t x, int32_t y, Rgb rgb) {
    ASSERT((x >= 0) && (x < this->width));
    ASSERT((y >= 0) && (y < this->height));
    this->pixels[(this->width * y) + x] = rgb;
}
