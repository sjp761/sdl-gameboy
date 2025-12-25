#pragma once
#include "SDL_SmartPointer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

class SDL_TileViewer
{
public:
    SDL_TileViewer();
    ~SDL_TileViewer();
    
    void init();
    void update(const uint8_t* vram);
    void render();
    void cleanup();
    
    bool is_open() const { return window != nullptr; }
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_SmartTexture tile_texture;
    TTF_Font* font;
    
    // Constants for display
    static constexpr int TILE_SIZE = 8;
    static constexpr int TILE_SCALE = 2;
    static constexpr int SCALED_TILE_SIZE = TILE_SIZE * TILE_SCALE;
    static constexpr int NUMBER_WIDTH = 20;  // Width for tile number display
    static constexpr int BOX_WIDTH = SCALED_TILE_SIZE + NUMBER_WIDTH;  // Total width per box
    static constexpr int TILES_PER_ROW = 24;
    static constexpr int TILES_PER_COL = 16;
    static constexpr int TOTAL_TILES = 384;
    
    void render_tile(const uint8_t* vram, int tile_index, int x, int y);
    uint32_t get_color(uint8_t pixel_value);
};
