#include "SDL_TileViewer.h"
#include <cstring>

SDL_TileViewer::SDL_TileViewer()
    : window(nullptr), renderer(nullptr)
{
}

SDL_TileViewer::~SDL_TileViewer()
{
    cleanup();
}

void SDL_TileViewer::init()
{
    // Calculate window dimensions for 24x16 tile grid
    int window_width = TILES_PER_ROW * SCALED_TILE_SIZE;
    int window_height = TILES_PER_COL * SCALED_TILE_SIZE;
    
    window = SDL_CreateWindow("VRAM Tile Viewer (384 Tiles)", 
                              window_width, 
                              window_height, 
                              SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        SDL_Log("Failed to create tile viewer window: %s", SDL_GetError());
        return;
    }
    
    renderer = SDL_CreateRenderer(window, nullptr);
    
    if (!renderer) {
        SDL_Log("Failed to create tile viewer renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        return;
    }
    
    // Create a texture for rendering individual tiles
    tile_texture.reset(SDL_CreateTexture(renderer, 
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        TILE_SIZE, 
                                        TILE_SIZE));
}

void SDL_TileViewer::cleanup()
{
    tile_texture.reset();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

uint32_t SDL_TileViewer::get_color(uint8_t pixel_value)
{
    // Game Boy color palette (RGBA format)
    switch (pixel_value & 0x03) {
        case 0: return 0xFFFFFFFF; // White
        case 1: return 0xFFAAAAAA; // Light gray
        case 2: return 0xFF555555; // Dark gray
        case 3: return 0xFF000000; // Black
        default: return 0xFFFFFFFF;
    }
}

void SDL_TileViewer::render_tile(const uint8_t* vram, int tile_index, int x, int y)
{
    if (!tile_texture.get() || !vram || tile_index >= TOTAL_TILES) return;
    
    // Each tile is 16 bytes (8x8 pixels, 2 bits per pixel)
    uint16_t tile_address = tile_index * 16;
    
    // Lock texture and get pixel buffer
    void* pixels_raw;
    int pitch;
    if (!SDL_LockTexture(tile_texture.get(), nullptr, &pixels_raw, &pitch)) {
        return;
    }
    
    uint32_t* pixels = static_cast<uint32_t*>(pixels_raw);
    
    // Decode tile data
    for (int row = 0; row < TILE_SIZE; row++) 
    {
        uint8_t byte1 = vram[tile_address + (row * 2)]; //Each pixel is represented by 2 bits across 2 bytes
        uint8_t byte2 = vram[tile_address + (row * 2) + 1];
        
        for (int col = 0; col < TILE_SIZE; col++)
        { // Process each pixel in the row
            int bit = 7 - col; // The left most bit is for the left pixel, shift it to get the color info
            uint8_t pixel_value = ((byte1 >> bit) & 1) | (((byte2 >> bit) & 1) << 1); //Each pixel is 2 bits for the color, the bits are swapped when determining the color so OR them to get the final value
            pixels[row * TILE_SIZE + col] = get_color(pixel_value); 
        }
    }
    
    SDL_UnlockTexture(tile_texture.get());
    
    // Render scaled tile
    SDL_FRect dest_rect = {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(SCALED_TILE_SIZE),
        static_cast<float>(SCALED_TILE_SIZE)
    };
    
    SDL_RenderTexture(renderer, tile_texture.get(), nullptr, &dest_rect);
    
    // Draw border around tile (subtle grid)
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderRect(renderer, &dest_rect);
}

void SDL_TileViewer::update(const uint8_t* vram)
{
    if (!window || !renderer || !vram) return;
    
    // Clear the window with dark background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    
    // Render all 384 tiles in a 24x16 grid
    for (int tile_idx = 0; tile_idx < TOTAL_TILES; tile_idx++) {
        int row = tile_idx / TILES_PER_ROW;
        int col = tile_idx % TILES_PER_ROW;
        
        int x = col * SCALED_TILE_SIZE;
        int y = row * SCALED_TILE_SIZE;
        
        render_tile(vram, tile_idx, x, y);
    }
}

void SDL_TileViewer::render()
{
    if (!renderer) return;
    SDL_RenderPresent(renderer);
}
