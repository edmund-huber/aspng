#include <chrono>
using namespace std::chrono;
#include <cmath>
#include <iostream>
#include <SDL2/SDL.h>

#include "aspng.h"
#include "common.h"
#include "png.h"
#include "simple_aspng_surface.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

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
    uint32_t *pixels;
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

Rgb SDLAspngSurface::get_pixel(int32_t, int32_t) {
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
    ASSERT(this->pitch % 4 == 0);
    this->pixels[((this->pitch / 4) * y) + x] = pixel;
}

void SDLAspngSurface::finish_draw(void) {
    ASSERT(this->sdl_texture != nullptr);
    SDL_UnlockTexture(this->sdl_texture);
    this->ready_to_draw = false;
}

// TODO: destructor should free stuff

class OneLoopContext {
    public:
        OneLoopContext(SDL_Renderer *r, Png *png, SDLAspngSurface *s, Aspng *a) :
            sdl_renderer(r),
            sdl_aspng_surface(s),
            aspng(a),
            input_map(png->get_width(), png->get_height()),
            pan_x(0),
            pan_y(0),
            zoom_level(0),
            zoom(std::pow(1.2, 0)),
            device_being_clicked(nullptr),
            do_quit(false),
            step_period(1000),
            since_last_step(0),
            since_last_frame(0)
            {}

        SDL_Renderer *sdl_renderer;
        SDLAspngSurface *sdl_aspng_surface;
        Aspng *aspng;
        SimpleAspngSurface input_map;
        double pan_x;
        double pan_y;
        int32_t zoom_level;
        double zoom;
        Coord last_mouse_position;
        std::shared_ptr<Device> device_being_clicked;
        bool do_quit;
        int32_t step_period;
        milliseconds since_last_step;
        milliseconds since_last_frame;
};

OneLoopContext *context = NULL;

void one_loop(void) {
    // Process all the SDL events queued up right now.
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {

        case SDL_KEYDOWN:
            switch(e.key.keysym.sym) {
            case SDLK_a:
                context->step_period = std::min(context->step_period + 10, 1000);
                break;
            case SDLK_z:
                context->step_period = std::max(context->step_period - 10, 0);
                break;
            case SDLK_ESCAPE:
                context->do_quit = true;
                return;
            default:
                break;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch (e.button.button) {
            case SDL_BUTTON_LEFT:
                {
                    Coord coord(
                        (e.button.x / context->zoom) - context->pan_x,
                        (e.button.y / context->zoom) - context->pan_y
                    );
                    auto device = context->aspng->which_device(coord);
                    if (device != nullptr) {
                        context->device_being_clicked = device;
                        context->device_being_clicked->click(coord);
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
                if (context->device_being_clicked != nullptr) {
                    context->device_being_clicked->unclick();
                    context->device_being_clicked = nullptr;
                }
                break;
            default:
                break;
            }
            break;

        case SDL_MOUSEMOTION:
            #ifndef __EMSCRIPTEN__
            context->last_mouse_position = Coord(e.motion.x, e.motion.y);
            switch (e.motion.state) {
            case SDL_BUTTON_MMASK:
                context->pan_x += e.motion.xrel / context->zoom;
                context->pan_y += e.motion.yrel / context->zoom;
                break;
            default:
                break;
            }
            #endif
            break;

        case SDL_MOUSEWHEEL:
            #ifndef __EMSCRIPTEN__
            {
                double mouse_pre_zoom_x = context->last_mouse_position.x / context->zoom;
                double mouse_pre_zoom_y = context->last_mouse_position.y / context->zoom;
                context->zoom_level += e.wheel.y;
                context->zoom = std::pow(1.2, context->zoom_level);
                double mouse_post_zoom_x = context->last_mouse_position.x / context->zoom;
                double mouse_post_zoom_y = context->last_mouse_position.y / context->zoom;
                context->pan_x += mouse_post_zoom_x - mouse_pre_zoom_x;
                context->pan_y += mouse_post_zoom_y - mouse_pre_zoom_y;
            }
            #endif
            break;

        case SDL_QUIT:
            context->do_quit = true;
            return;
        }
    }


    // If it's time to do a simulation step, then let's do that.
    milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    if (now - context->since_last_step >= milliseconds(context->step_period)) {
        context->since_last_step = now;

        try {
            context->aspng->step();
        } catch (AspngSimException &e) {
            std::cout << "error: " << e.message << std::endl;
            exit(1);
        }
    }

    // If it's time to render a frame, then let's do that.
    now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    if (now - context->since_last_frame >= milliseconds(1000 / 60)) {
        context->since_last_frame = now;

        // Render the aspng to a texture.
        SDL_SetRenderDrawColor(context->sdl_renderer, 255, 0, 255, 255);
        SDL_RenderClear(context->sdl_renderer);
        context->sdl_aspng_surface->start_draw();
        context->aspng->draw(context->sdl_aspng_surface);

        // Blit that texture to the SDL window, applying zoom and pan.
        context->sdl_aspng_surface->finish_draw();
        SDL_Rect sdl_rect_dst;
        sdl_rect_dst.w = context->sdl_aspng_surface->get_width() * context->zoom;
        sdl_rect_dst.h = context->sdl_aspng_surface->get_height() * context->zoom;
        sdl_rect_dst.x = context->pan_x * context->zoom;
        sdl_rect_dst.y = context->pan_y * context->zoom;
        SDL_RenderCopy(context->sdl_renderer, context->sdl_aspng_surface->get_texture(), nullptr, &sdl_rect_dst);
        SDL_RenderPresent(context->sdl_renderer);
    }
}

int main(int argc, char **argv) {
    // There should be just one argument: the png input.
    if (argc != 2) {
        return -1;
    }
    auto png = Png::read(argv[1]);
    if (png == nullptr) {
        std::cout << "can't read input" << std::endl;
        return -1;
    }
    std::string error;
    auto aspng = new Aspng(png, error);
    if (error != "") {
        std::cout << "creating Aspng object failed: " << error << std::endl;
        return -1;
    }

    // Create the SDL window and renderer.
    ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    ASSERT(SDL_CreateWindowAndRenderer(png->get_width(), png->get_height(), 0, &sdl_window, &sdl_renderer) == 0);
    auto sdl_aspng_surface = new SDLAspngSurface(sdl_renderer, png->get_width(), png->get_height());

    // Event loop!
    context = new OneLoopContext(sdl_renderer, png, sdl_aspng_surface, aspng);
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(&one_loop, 0, 0);
        return 0;
    #else
        while (!context->do_quit) {
            one_loop();
            // It's incumbent on one_loop to pause an appropriate amount of
            // time, so no sleep() here.
        }
        SDL_Quit();
        return 0;
    #endif
}
