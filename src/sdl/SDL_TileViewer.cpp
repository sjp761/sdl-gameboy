#include "SDL_TileViewer.h"
#include <cstring>

SDL_TileViewer::SDL_TileViewer()
    : window(nullptr), renderer(nullptr), font(nullptr)
{
}

SDL_TileViewer::~SDL_TileViewer()
{
    cleanup();
}

void SDL_TileViewer::init()
{
    // Initialize SDL_ttf
    if (!TTF_WasInit()) {
        TTF_Init();
    }
    
    // Load a system font (try to use a common system font)
    font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 10);
    if (!font) {
        // Fallback to another common font if Helvetica not found
        font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 10);
    }
    if (!font) {
        SDL_Log("Warning: Could not load font, tile numbers will not be displayed");
    }
    
    // Calculate window dimensions for 24x16 tile grid with number display
    int window_width = TileViewerConstants::TILES_PER_ROW * TileViewerConstants::BOX_WIDTH;
    int window_height = TileViewerConstants::TILES_PER_COL * TileViewerConstants::SCALED_TILE_SIZE;
    
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
                                        TileViewerConstants::TILE_SIZE, 
                                        TileViewerConstants::TILE_SIZE));
}

void SDL_TileViewer::cleanup()
{
    tile_texture.reset();
    
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
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
    if (!tile_texture.get() || !vram || tile_index >= TileViewerConstants::TOTAL_TILES) return;
    
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
    for (int row = 0; row < TileViewerConstants::TILE_SIZE; row++) 
    {
        uint8_t byte1 = vram[tile_address + (row * 2)]; //Each pixel is represented by 2 bits across 2 bytes
        uint8_t byte2 = vram[tile_address + (row * 2) + 1];
        
        for (int col = 0; col < TileViewerConstants::TILE_SIZE; col++)
        { // Process each pixel in the row
            int bit = 7 - col; // The left most bit is for the left pixel, shift it to get the color info
            uint8_t pixel_value = ((byte1 >> bit) & 1) | (((byte2 >> bit) & 1) << 1); //Each pixel is 2 bits for the color, the bits are swapped when determining the color so OR them to get the final value
            pixels[row * TileViewerConstants::TILE_SIZE + col] = get_color(pixel_value); 
        }
    }
    
    SDL_UnlockTexture(tile_texture.get());
    
    // Render tile sprite on the right side
    SDL_FRect tile_dest_rect = {
        static_cast<float>(x + TileViewerConstants::NUMBER_WIDTH),
        static_cast<float>(y),
        static_cast<float>(TileViewerConstants::SCALED_TILE_SIZE),
        static_cast<float>(TileViewerConstants::SCALED_TILE_SIZE)
    };
    SDL_RenderTexture(renderer, tile_texture.get(), nullptr, &tile_dest_rect);
    
    // Draw border around tile
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderRect(renderer, &tile_dest_rect);
    
    // Render tile number on the left if font is available
    if (font) {
        char tile_number_str[16];
        snprintf(tile_number_str, sizeof(tile_number_str), "%d", tile_index);
        
        SDL_Color text_color = {255, 255, 255, 255}; // White text
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, tile_number_str, 0, text_color);
        
        if (text_surface) {
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            if (text_texture) {
                int text_width = text_surface->w;
                int text_height = text_surface->h;
                
                // Center text vertically and position it on the left side
                SDL_FRect text_rect = {
                    static_cast<float>(x + (TileViewerConstants::NUMBER_WIDTH - text_width) / 2),
                    static_cast<float>(y + (TileViewerConstants::SCALED_TILE_SIZE - text_height) / 2),
                    static_cast<float>(text_width),
                    static_cast<float>(text_height)
                };
                
                SDL_RenderTexture(renderer, text_texture, nullptr, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            
            SDL_DestroySurface(text_surface);
        }
    }
}

void SDL_TileViewer::update(const uint8_t* vram)
{
    if (!window || !renderer || !vram) return;
    
    // Clear the window with dark background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    
    // Render all 384 tiles in a 24x16 grid
    for (int tile_idx = 0; tile_idx < TileViewerConstants::TOTAL_TILES; tile_idx++) {
        int row = tile_idx / TileViewerConstants::TILES_PER_ROW;
        int col = tile_idx % TileViewerConstants::TILES_PER_ROW;
        
        int x = col * TileViewerConstants::BOX_WIDTH;
        int y = row * TileViewerConstants::SCALED_TILE_SIZE;
        
        render_tile(vram, tile_idx, x, y);
    }
}

void SDL_TileViewer::render()
{
    if (!renderer) return;
    SDL_RenderPresent(renderer);
}
