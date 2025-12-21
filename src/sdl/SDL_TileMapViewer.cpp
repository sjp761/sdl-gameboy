#include "SDL_TileMapViewer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>


SDL_TileMapViewer::SDL_TileMapViewer()
    : window(nullptr), renderer(nullptr), font(nullptr)
{
    if (!TTF_Init()) {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
    }
}


SDL_TileMapViewer::~SDL_TileMapViewer()
{
    cleanup();
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
}


void SDL_TileMapViewer::init()
{
    int window_width = MAP_COLS * SCALED_TILE_SIZE * 2; // Double width for both tile maps
    int window_height = MAP_ROWS * SCALED_TILE_SIZE;
    window = SDL_CreateWindow("Tile Map Viewer (Both Maps)", window_width, window_height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create tile map viewer window: %s", SDL_GetError());
        return;
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("Failed to create tile map viewer renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        return;
    }
    // Load Arial font for tile numbers (no Qt logic)
    // Try common macOS font paths
    const char* fontPaths[] = {
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Microsoft/Arial.ttf"
    };
    
    for (const char* path : fontPaths) {
        font = TTF_OpenFont(path, 12);
        if (font) {
            SDL_Log("Loaded font from: %s", path);
            break;
        }
    }
    
    if (!font) {
        SDL_Log("Failed to load Arial font for tile map viewer (will render without text): %s", SDL_GetError());
    }
}


void SDL_TileMapViewer::cleanup()
{
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}


void SDL_TileMapViewer::update(const uint8_t* tilemap)
{
    if (!window || !renderer || !tilemap) return;
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White background
    SDL_RenderClear(renderer);
    
    // Render both tile maps side by side
    // Left side: tile_map_1 (passed as tilemap)
    // Right side: tile_map_2 (tilemap + 0x400)
    for (int map_idx = 0; map_idx < 2; ++map_idx) {
        int x_offset = map_idx * MAP_COLS * SCALED_TILE_SIZE;
        const uint8_t* current_map = tilemap + (map_idx * 0x400);
        
        for (int idx = 0; idx < MAP_SIZE; ++idx) {
            int row = idx / MAP_COLS;
            int col = idx % MAP_COLS;
            int x = x_offset + col * SCALED_TILE_SIZE;
            int y = row * SCALED_TILE_SIZE;
            
            SDL_FRect rect = { static_cast<float>(x), static_cast<float>(y), 
                             static_cast<float>(SCALED_TILE_SIZE), static_cast<float>(SCALED_TILE_SIZE) };
            
            // Background color
            SDL_SetRenderDrawColor(renderer, 80, 80, 160, 255);
            SDL_RenderFillRect(renderer, &rect);
            
            // Border
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderRect(renderer, &rect);

            // Render tile number as text using SDL_ttf (if font is available)
            if (font) {
                char numStr[4];
                snprintf(numStr, sizeof(numStr), "%d", current_map[idx]);
                SDL_Color textColor = {255, 255, 255, 255};
                SDL_Surface* textSurface = TTF_RenderText_Blended(font, numStr, 0, textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        int tw = textSurface->w;
                        int th = textSurface->h;
                        SDL_FRect textRect = {rect.x + (rect.w-tw)/2, rect.y + (rect.h-th)/2, (float)tw, (float)th};
                        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_DestroySurface(textSurface);
                }
            }
        }
    }
}


void SDL_TileMapViewer::render()
{
    if (!renderer) return;
    SDL_RenderPresent(renderer);
}
