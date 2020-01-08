#include <iostream>
#include <SDL2/SDL.h>

#include "aspng.h"
#include "common.h"
#include "png.h"
#include "simple_aspng_surface.h"

class SDLAspngSurface : public AspngSurface {
public:
    SDLAspngSurface(SDL_Renderer *, size_t, size_t);
    size_t get_width(void);
    size_t get_height(void);
    Rgb get_pixel(size_t, size_t);
    void set_pixel(size_t, size_t, Rgb);
    Patch get_patch_at(size_t, size_t, size_t, size_t);
    SDL_Texture *get_texture(void);

private:
    SDL_Texture *sdl_texture;
};

SDLAspngSurface::SDLAspngSurface(SDL_Renderer *sdl_renderer, size_t w, size_t h) {
    this->sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    ASSERT(this->sdl_texture != nullptr);
}

size_t SDLAspngSurface::get_width(void) {
    int w, h;
    SDL_QueryTexture(this->sdl_texture, nullptr, nullptr, &w, &h);
    return w;
}

size_t SDLAspngSurface::get_height(void) {
    int w, h;
    SDL_QueryTexture(this->sdl_texture, nullptr, nullptr, &w, &h);
    return h;
}

SDL_Texture *SDLAspngSurface::get_texture(void) {
    return this->sdl_texture;
}

Rgb SDLAspngSurface::get_pixel(size_t x, size_t y) {
    // SDLAspngSurface is really only meant as an output surface.
    ASSERT(0);
}

void SDLAspngSurface::set_pixel(size_t x, size_t y, Rgb rgb) {
    // Get information about the texture and do bounds-checking.
    int w, h;
    uint32_t format;
    SDL_QueryTexture(this->sdl_texture, &format, nullptr, &w, &h);
    SDL_PixelFormat *sdl_pixel_format = SDL_AllocFormat(format);
    ASSERT((x >= 0) && ((int)x < w));
    ASSERT((y >= 0) && ((int)y < h));

    uint8_t *pixels;
    int pitch;
    ASSERT(SDL_LockTexture(this->sdl_texture, nullptr, (void **)&pixels, &pitch) == 0);
    ASSERT(pixels != nullptr);
    uint32_t pixel = SDL_MapRGBA(sdl_pixel_format, rgb.r, rgb.g, rgb.b, 255);
    *((uint32_t *)&pixels[(pitch * y) + (4 * x)]) = pixel;
    SDL_UnlockTexture(this->sdl_texture);
    SDL_FreeFormat(sdl_pixel_format);
}

// TODO: destructor should free stuff

int main(int argc, char **argv) {
    if (argc != 2) {
        return -1;
    }

    // Create the SDL window and renderer.
    ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);
    SDL_Window *sdl_window = SDL_CreateWindow("aspng", 0, 0, 100, 100, SDL_WINDOW_RESIZABLE);
    ASSERT(sdl_window != nullptr);
    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT(sdl_renderer != nullptr);

    auto png = Png::read(argv[1]);
    if (png == nullptr)
        return -1;
    SDLAspngSurface sdl_aspng_surface(sdl_renderer, png->get_width(), png->get_height());

    std::string error;
    auto aspng = Aspng(png, error);
    if (error != "") {
        std::cout << "creating Aspng object failed: " << error << std::endl;
        return -1;
    }

    // Event loop!
    SimpleAspngSurface input_map(png->get_width(), png->get_height());
    double pan_x = 0;
    double pan_y = 0;
    int32_t zoom_level = 2;
    double zoom = pow(2, zoom_level);
    bool do_quit = false;
    std::shared_ptr<Device> device_being_clicked = nullptr;
    while (!do_quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {

            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    do_quit = true;
                    break;

                default:
                    break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch (e.button.button) {
                case SDL_BUTTON_LEFT:
                    {
                        Coord coord(
                            (e.button.x / zoom) - pan_x,
                            (e.button.y / zoom) - pan_y
                        );
                        auto device = aspng.which_device(coord);
                        if (device != nullptr) {
                            device_being_clicked = device;
                            device_being_clicked->click(coord);
                        }
                    }
                    break;

                default:
                    break;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                switch (e.button.button) {
                case SDL_BUTTON_LEFT:
                    if (device_being_clicked != nullptr) {
                        device_being_clicked->unclick();
                        device_being_clicked = nullptr;
                    }
                    break;

                default:
                    break;
                }

            case SDL_MOUSEMOTION:
                switch (e.motion.state) {
                case SDL_BUTTON_RMASK:
                    pan_x += e.motion.xrel / zoom;
                    pan_y += e.motion.yrel / zoom;
                    break;

                default:
                    break;
                }
                break;

            case SDL_MOUSEWHEEL:
                zoom_level += e.wheel.y;
                zoom = pow(2, zoom_level);
                break;

            case SDL_QUIT:
                do_quit = true;
                break;

            default:
                break;
            }
        }

        SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 255, 255);
        SDL_RenderClear(sdl_renderer);
        aspng.draw(&sdl_aspng_surface);
        aspng.step();

        SDL_Rect sdl_rect_dst;
        sdl_rect_dst.w = sdl_aspng_surface.get_width() * zoom;
        sdl_rect_dst.h = sdl_aspng_surface.get_height() * zoom;
        sdl_rect_dst.x = pan_x * zoom;
        sdl_rect_dst.y = pan_y * zoom;
        SDL_RenderCopy(sdl_renderer, sdl_aspng_surface.get_texture(), nullptr, &sdl_rect_dst);
        SDL_RenderPresent(sdl_renderer);

        // TODO
        SDL_Delay(5);
    }

    SDL_Quit();
    return 0;
}
