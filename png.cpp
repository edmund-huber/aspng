#include "common.h"
#include "png.h"

Png::Png() {}

// Cribbed from https://gist.github.com/niw/5963798 .
Png *Png::open(std::string fn) {
    FILE *f = fopen(fn.c_str(), "r");
    ASSERT(f != NULL);
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
    for (int y = 0; y < p->height; y++) {
        p->rows[y] = new png_byte[png_get_rowbytes(png, info)];
    }
    png_read_image(png, p->rows);
    fclose(f);
    return p;
}
