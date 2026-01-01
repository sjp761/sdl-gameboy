#pragma once
#include "SDL_SmartPointer.h"
#include "tile_viewer_constants.h"
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
    
    void render_tile(const uint8_t* vram, int tile_index, int x, int y);
    uint32_t get_color(uint8_t pixel_value);
};
