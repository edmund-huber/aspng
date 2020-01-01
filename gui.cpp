#include <SDL2/SDL.h>

#include "common.h"

int main(int argc, char **argv) {
    // Create the SDL window and renderer.
    ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);
    SDL_Window *sdl_window = SDL_CreateWindow("aspng", 0, 0, 100, 100, SDL_WINDOW_RESIZABLE);
    ASSERT(sdl_window != nullptr);
    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT(sdl_renderer != nullptr);

    // Event loop!
    bool do_quit = false;
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

            case SDL_QUIT:
                do_quit = true;
                break;

            default:
                break;
            }
        }

        SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 255, 255);
        SDL_RenderClear(sdl_renderer);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_Quit();
    return 0;
}
