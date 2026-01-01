#pragma once

namespace TileViewerConstants {
    // Tile rendering
    constexpr int TILE_SIZE = 8; // Pixels per tile (8x8)
    constexpr int TILE_SCALE = 2; // Scaling factor for display
    constexpr int SCALED_TILE_SIZE = TILE_SIZE * TILE_SCALE;
    
    // Tile Viewer specific
    constexpr int TILES_PER_ROW = 24;
    constexpr int TILES_PER_COL = 16;
    constexpr int TOTAL_TILES = 384;
    constexpr int NUMBER_WIDTH = 20;  // Width for tile number display
    constexpr int BOX_WIDTH = SCALED_TILE_SIZE + NUMBER_WIDTH;
    
    // Tile Map Viewer specific
    constexpr int MAP_COLS = 32;
    constexpr int MAP_ROWS = 32;
    constexpr int MAP_SIZE = MAP_COLS * MAP_ROWS;
    constexpr int HEADER_HEIGHT = 30;  // Height for address header
    constexpr int BAR_WIDTH = 30;      // Width of separator bar between maps
}
