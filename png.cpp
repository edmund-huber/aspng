#include <string.h>

#include "common.h"
#include "png.h"

bool Rgb::operator==(Rgb &other) {
    return (this->r == other.r) && (this->g == other.g) && (this->b == other.b);
}

Png::Png() {}

// Cribbed from https://gist.github.com/niw/5963798 .
Png *Png::read(std::string fn) {
    FILE *f = fopen(fn.c_str(), "r");
    if (f == NULL) {
        return NULL;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    ASSERT(png != NULL);
    png_infop info = png_create_info_struct(png);
    ASSERT(info != NULL);
    if (setjmp(png_jmpbuf(png))) {
        ASSERT(0);
    }
    png_init_io(png, f);
    png_read_info(png, info);
    Png *p = new Png();
    p->width = png_get_image_width(png, info);
    p->height = png_get_image_height(png, info);
    png_byte depth = png_get_bit_depth(png, info);
    if (depth == 16) {
        png_set_strip_16(png);
    }
    png_byte color_type = png_get_color_type(png, info);
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if ((color_type == PNG_COLOR_TYPE_RGB) ||
        (color_type == PNG_COLOR_TYPE_GRAY) ||
        (color_type == PNG_COLOR_TYPE_PALETTE)) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }
    if ((color_type == PNG_COLOR_TYPE_GRAY) ||
        (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) {
        png_set_gray_to_rgb(png);
    }
    png_read_update_info(png, info);
    p->rows = new png_bytep[p->height];
    for (size_t y = 0; y < p->height; y++) {
        p->rows[y] = new png_byte[png_get_rowbytes(png, info)];
    }
    png_read_image(png, p->rows);
    fclose(f);
    return p;
}

size_t Png::get_width(void) {
    return this->width;
}

size_t Png::get_height(void) {
    return this->height;
}

Rgb Png::get_pixel(size_t x, size_t y) {
    png_bytep pixel = &(this->rows[y][x * 4]);
    return Rgb(pixel[0], pixel[1], pixel[2]);
}

void Png::set_pixel(size_t x, size_t y, Rgb rgb) {
    png_bytep pixel = &(this->rows[y][x * 4]);
    pixel[0] = rgb.r;
    pixel[1] = rgb.g;
    pixel[2] = rgb.b;
}

// Cribbed from https://gist.github.com/niw/5963798 .
void Png::write(std::string fn) {
    FILE *f = fopen(fn.c_str(), "wb");
    ASSERT(f != NULL);

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    ASSERT(png != NULL);

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) {
        ASSERT(0);
    }
    png_init_io(png, f);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
        png,
        info,
        this->width, this->height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_write_image(png, rows);
    png_write_end(png, NULL);

    for (size_t y = 0; y < height; y++) {
        free(this->rows[y]);
    }
    free(this->rows);

    fclose(f);

    png_destroy_write_struct(&png, &info);
}
