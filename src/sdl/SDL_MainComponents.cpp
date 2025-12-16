#include "SDL_MainComponents.h"
#include "Configuration.h"
#include <tuple>

SDL_Window* SDL_MainComponents::window;
SDL_Renderer* SDL_MainComponents::renderer;

SDL_SmartPointer<SDL_Texture> SDL_MainComponents::display;
SDL_TileViewer SDL_MainComponents::tile_viewer;

void SDL_MainComponents::renderUpdate()
{
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, display.get(), NULL, NULL);
    SDL_RenderPresent(renderer);
}

void SDL_MainComponents::init()
{
    window = SDL_CreateWindow("SDL Window", configuration::WINDOW_WIDTH * configuration::SCALE_FACTOR, configuration::WINDOW_HEIGHT * configuration::SCALE_FACTOR, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, nullptr);
}

void SDL_MainComponents::initDebugWindows()
{
    tile_viewer.init();
}