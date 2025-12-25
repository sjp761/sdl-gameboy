#pragma once
#include "SDL_SmartPointer.h"
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

    // Constants for display
    static constexpr int TILE_SIZE = 8;
    static constexpr int TILE_SCALE = 2;
    static constexpr int SCALED_TILE_SIZE = TILE_SIZE * TILE_SCALE;
    static constexpr int MAP_COLS = 32;
    static constexpr int MAP_ROWS = 32;
    static constexpr int MAP_SIZE = MAP_COLS * MAP_ROWS;
    static constexpr int HEADER_HEIGHT = 30;  // Height for address header
    static constexpr int BAR_WIDTH = 30;      // Width of separator bar between maps
};
