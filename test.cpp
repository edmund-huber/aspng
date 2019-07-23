#include <iostream>

#include "png.h"

int main(void) {
    Png *png = Png::open("tests/basic_source_sink/_.png");
    for (int y = 0; y < png->height; y++) {
        for (int x = 0; x < png->width; x++) {
            png_bytep pixel = &png->rows[y][x * 4];
            std::cout
                << static_cast<int>(pixel[0]) << " "
                << static_cast<int>(pixel[1]) << " "
                << static_cast<int>(pixel[2]) << " "
                << static_cast<int>(pixel[3]) << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
