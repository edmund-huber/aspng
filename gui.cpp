#include <chrono>
using namespace std::chrono;
#include <iostream>
#include <SDL2/SDL.h>

#include "aspng.h"
#include "common.h"
#include "png.h"
#include "simple_aspng_surface.h"

class SDLAspngSurface : public AspngSurface {
public:
    SDLAspngSurface(SDL_Renderer *, int32_t, int32_t);
    int32_t get_width(void);
    int32_t get_height(void);
    Rgb get_pixel(int32_t, int32_t);
    void set_pixel(int32_t, int32_t, Rgb);
    void start_draw(void);
    void finish_draw(void);
    Patch get_patch_at(int32_t, int32_t, int32_t, int32_t);
    SDL_Texture *get_texture(void);

private:
    SDL_Texture *sdl_texture;
    SDL_PixelFormat *sdl_pixel_format;
    int32_t width;
    int32_t height;
    uint8_t *pixels;
    int pitch;
    bool ready_to_draw;
};

SDLAspngSurface::SDLAspngSurface(SDL_Renderer *sdl_renderer, int32_t w, int32_t h) {
    this->sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    ASSERT(this->sdl_texture != nullptr);
    uint32_t format;
    SDL_QueryTexture(this->sdl_texture, &format, nullptr, nullptr, nullptr);
    this->sdl_pixel_format = SDL_AllocFormat(format);
    ASSERT(this->sdl_pixel_format != nullptr);
    this->width = w;
    this->height = h;
    this->ready_to_draw = false;
}

int32_t SDLAspngSurface::get_width(void) {
    return this->width;
}

int32_t SDLAspngSurface::get_height(void) {
    return this->height;
}

SDL_Texture *SDLAspngSurface::get_texture(void) {
    return this->sdl_texture;
}

Rgb SDLAspngSurface::get_pixel(int32_t x, int32_t y) {
    // SDLAspngSurface is really only meant as an output surface.
    ASSERT(0);
}

void SDLAspngSurface::start_draw(void) {
    ASSERT(SDL_LockTexture(this->sdl_texture, nullptr, (void **)&(this->pixels), &(this->pitch)) == 0);
    this->ready_to_draw = true;
}

void SDLAspngSurface::set_pixel(int32_t x, int32_t y, Rgb rgb) {
    ASSERT(this->ready_to_draw);
    ASSERT((x >= 0) && (x < this->width));
    ASSERT((y >= 0) && (y < this->height));
    ASSERT(this->pixels != nullptr);
    uint32_t pixel = SDL_MapRGBA(this->sdl_pixel_format, rgb.r, rgb.g, rgb.b, 255);
    *((uint32_t *)&(this->pixels)[(this->pitch * y) + (4 * x)]) = pixel;
}

void SDLAspngSurface::finish_draw(void) {
    SDL_UnlockTexture(this->sdl_texture);
    this->ready_to_draw = false;
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
    Coord last_mouse_position;
    bool do_quit = false;
    std::shared_ptr<Device> device_being_clicked = nullptr;
    milliseconds t0 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    int frames = 0;
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
                last_mouse_position = Coord(e.motion.x, e.motion.y);
                switch (e.motion.state) {
                case SDL_BUTTON_MMASK:
                    pan_x += e.motion.xrel / zoom;
                    pan_y += e.motion.yrel / zoom;
                    break;

                default:
                    break;
                }
                break;

            case SDL_MOUSEWHEEL:
                {
                    double mouse_pre_zoom_x = last_mouse_position.x / zoom;
                    double mouse_pre_zoom_y = last_mouse_position.y / zoom;
                    zoom_level += e.wheel.y;
                    zoom = pow(1.2, zoom_level);
                    double mouse_post_zoom_x = last_mouse_position.x / zoom;
                    double mouse_post_zoom_y = last_mouse_position.y / zoom;
                    pan_x += mouse_post_zoom_x - mouse_pre_zoom_x;
                    pan_y += mouse_post_zoom_y - mouse_pre_zoom_y;
                }
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
        sdl_aspng_surface.start_draw();
        aspng.draw(&sdl_aspng_surface);
        sdl_aspng_surface.finish_draw();
        aspng.step();

        SDL_Rect sdl_rect_dst;
        sdl_rect_dst.w = sdl_aspng_surface.get_width() * zoom;
        sdl_rect_dst.h = sdl_aspng_surface.get_height() * zoom;
        sdl_rect_dst.x = pan_x * zoom;
        sdl_rect_dst.y = pan_y * zoom;
        SDL_RenderCopy(sdl_renderer, sdl_aspng_surface.get_texture(), nullptr, &sdl_rect_dst);
        SDL_RenderPresent(sdl_renderer);

        milliseconds t1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        int elapsed = (t1 - t0).count();
        if (elapsed >= 1000) {
            //std::cout << (frames / (elapsed / 1000.)) << std::endl;
            t0 = t1;
            frames = 0;
        }
        frames++;

        // TODO
        SDL_Delay(5);
    }

    SDL_Quit();
    return 0;
}
