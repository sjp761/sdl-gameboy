#pragma once
#include <cstdint>
#include <mutex>
class Bus;
class Cpu;
class LCD;
constexpr int SCANLINES_PER_FRAME = 154;
constexpr int VISIBLE_SCANLINES = 144;
constexpr uint16_t DOTS_PER_SCANLINE = 456; //uint16_t to avoid overflow
constexpr int OAM_SEARCH_DOTS = 80;
constexpr int PIXEL_TRANSFER_DOTS = 172;
constexpr int HBLANK_DOTS = 204;

constexpr int TILE_MAP_WIDTH = 32;
constexpr int TILE_MAP_HEIGHT = 32;
constexpr int VRAM_SIZE = 0x2000; // 8KB
constexpr int SCREEN_WIDTH = 160;
constexpr int SCREEN_HEIGHT = 144;
constexpr int SCREEN_BUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

struct oam_entry
{
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attributes;
};

// VRAM layout struct - named sections for organized access
struct vram_layout
{
    uint8_t tile_data_0[0x0800];      // 0x8000-0x87FF (2KB) - Block 0
    uint8_t tile_data_1[0x0800];      // 0x8800-0x8FFF (2KB) - Block 1
    uint8_t tile_data_2[0x0800];      // 0x9000-0x97FF (2KB) - Block 2
    uint8_t tile_map_1[0x0400];      // 0x9800-0x9BFF (1KB) -  Tile Map 1
    uint8_t tile_map_2[0x0400];  // 0x9C00-0x9FFF (1KB) -  Tile Map 2
};

class Ppu
{
public:
    Bus* bus;
    LCD* lcd;
    Cpu* cpu;
    oam_entry oam[40] = {};
    Ppu();
    uint8_t dot = 0; //Dot is current cycle within a scanline (as referred to in the Pandocs)
    void set_cmp(Bus* bus_ptr, LCD* lcd_ptr, Cpu* cpu_ptr);
    void ppu_tick();

    // Thread-safe screen buffer access
    void copy_screen_buffer(uint8_t* dest) const;

    // VRAM accessors (thread-safe)
    uint8_t vram_read(uint16_t address);
    void vram_write(uint16_t address, uint8_t value);
    
    // VRAM direct access (caller must lock get_vram_mutex())
    uint8_t* get_vram_ptr() { return reinterpret_cast<uint8_t*>(&vram); }
    uint8_t* get_tile_data_ptr() { return vram.tile_data_0; }
    uint8_t* get_tilemap_ptr() { return vram.tile_map_1; } // Default to tile_map_1; can be extended for bank selection
    std::mutex& get_vram_mutex() { return vram_mutex; }

    // OAM accessors
    uint8_t oam_read(uint16_t address);
    void oam_write(uint16_t address, uint8_t value);

private:
    // Video RAM (0x8000-0x9FFF)
    vram_layout vram;
    
    // Screen buffer (protected by mutex)
    uint8_t screen[SCREEN_BUFFER_SIZE] = {};
    mutable std::mutex screen_mutex;
    
    // VRAM mutex
    mutable std::mutex vram_mutex;

    void handle_oam_search();
    void handle_pixel_transfer();
    void handle_hblank();
    void handle_vblank();
};