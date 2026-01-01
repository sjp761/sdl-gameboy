#pragma once
#include <cstdint>

namespace PpuConstants {
    // Scanline and frame timing
    constexpr int SCANLINES_PER_FRAME = 154;
    constexpr int VISIBLE_SCANLINES = 144;
    
    // Dots (cycles) per phase
    constexpr uint16_t DOTS_PER_SCANLINE = 456; // uint16_t to avoid overflow
    constexpr int OAM_SEARCH_DOTS = 80;
    constexpr int PIXEL_TRANSFER_DOTS = 172;
    constexpr int HBLANK_DOTS = 204;
    
    // Tile dimensions
    constexpr int TILE_MAP_WIDTH = 32;
    constexpr int TILE_MAP_HEIGHT = 32;
    
    // Memory sizes
    constexpr int VRAM_SIZE = 0x2000; // 8KB
    
    // Screen dimensions
    constexpr int SCREEN_WIDTH = 160;
    constexpr int SCREEN_HEIGHT = 144;
    constexpr int SCREEN_BUFFER_SIZE = 160 * 144;
}
