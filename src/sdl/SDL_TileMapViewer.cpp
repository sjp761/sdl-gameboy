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
    int map_width = MAP_COLS * SCALED_TILE_SIZE;
    int map_height = MAP_ROWS * SCALED_TILE_SIZE;
    int window_width = map_width * 2 + BAR_WIDTH;  // Two maps plus separator bar
    int window_height = map_height + HEADER_HEIGHT;  // Add space for header
    
    window = SDL_CreateWindow("Tile Map Viewer (0x9800 | 0x9C00)", window_width, window_height, SDL_WINDOW_RESIZABLE);
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
    
    int map_width = MAP_COLS * SCALED_TILE_SIZE;
    int map_height = MAP_ROWS * SCALED_TILE_SIZE;
    int map_y_offset = HEADER_HEIGHT;
    int bar_x = map_width;  // Bar position between maps
    
    // Render headers
    if (font) {
        SDL_Color headerColor = {0, 0, 0, 255}; // Black text
        
        // Left map header: 0x9800
        SDL_Surface* leftHeaderSurface = TTF_RenderText_Blended(font, "0x9800", 0, headerColor);
        if (leftHeaderSurface) {
            SDL_Texture* leftHeaderTexture = SDL_CreateTextureFromSurface(renderer, leftHeaderSurface);
            if (leftHeaderTexture) {
                SDL_FRect leftHeaderRect = {10.0f, 5.0f, (float)leftHeaderSurface->w, (float)leftHeaderSurface->h};
                SDL_RenderTexture(renderer, leftHeaderTexture, nullptr, &leftHeaderRect);
                SDL_DestroyTexture(leftHeaderTexture);
            }
            SDL_DestroySurface(leftHeaderSurface);
        }
        
        // Right map header: 0x9C00
        SDL_Surface* rightHeaderSurface = TTF_RenderText_Blended(font, "0x9C00", 0, headerColor);
        if (rightHeaderSurface) {
            SDL_Texture* rightHeaderTexture = SDL_CreateTextureFromSurface(renderer, rightHeaderSurface);
            if (rightHeaderTexture) {
                SDL_FRect rightHeaderRect = {static_cast<float>(map_width + BAR_WIDTH + 10), 5.0f, (float)rightHeaderSurface->w, (float)rightHeaderSurface->h};
                SDL_RenderTexture(renderer, rightHeaderTexture, nullptr, &rightHeaderRect);
                SDL_DestroyTexture(rightHeaderTexture);
            }
            SDL_DestroySurface(rightHeaderSurface);
        }
    }
    
    // Render separator bar
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Gray bar
    SDL_FRect barRect = {static_cast<float>(bar_x), static_cast<float>(map_y_offset), 
                         static_cast<float>(BAR_WIDTH), static_cast<float>(map_height)};
    SDL_RenderFillRect(renderer, &barRect);
    
    // Render both tile maps side by side
    // Left side: tile_map_1 (passed as tilemap)
    // Right side: tile_map_2 (tilemap + 0x400)
    for (int map_idx = 0; map_idx < 2; ++map_idx) {
        int x_offset = (map_idx == 0) ? 0 : (map_width + BAR_WIDTH);
        const uint8_t* current_map = tilemap + (map_idx * 0x400);
        
        for (int idx = 0; idx < MAP_SIZE; ++idx) {
            int row = idx / MAP_COLS;
            int col = idx % MAP_COLS;
            int x = x_offset + col * SCALED_TILE_SIZE;
            int y = map_y_offset + row * SCALED_TILE_SIZE;
            
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
