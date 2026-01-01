#pragma once
#include "SDL_SmartPointer.h"
#include "tile_viewer_constants.h"
#include <SDL3/SDL.h>
#include <cstdint>

class TTF_Font;

class SDL_TileMapViewer
{
public:
    SDL_TileMapViewer();
    ~SDL_TileMapViewer();

    void init();
    void update(const uint8_t* tilemap);
    void render();
    void cleanup();

    bool is_open() const { return window != nullptr; }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
};
