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
constexpr int SCREEN_BUFFER_SIZE = 160*144; // The display holds 360 tiles of 16 bytes each (160x144 pixels, 8 pixels per byte, 2 bytes per tile row)
//Temporarily setting this to make things easier for me, will optimize later, original value is 360*16
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

struct scanline_state_t
{
    uint8_t scx;
    uint8_t scy;
    uint8_t ly;
    uint8_t wx;
    uint8_t wy;
    uint8_t window_line_counter; //Counts which line of the window is being drawn
    bool wy_condition; //Whether window is currently being drawn on this scanline
    bool wx_condition; //Whether window X position has been reached on this scanline
    bool background_enabled; //Based on LCDC bit 0
};

class Ppu
{
public:
    Bus* bus;
    LCD* lcd;
    Cpu* cpu;
    oam_entry oam[40] = {};
    Ppu();
    uint16_t dot = 0; //Dot is current cycle within a scanline (as referred to in the Pandocs)
    void set_cmp(Bus* bus_ptr, LCD* lcd_ptr, Cpu* cpu_ptr);
    void ppu_tick();

    // Double-buffered access - rendering thread gets front buffer
    const uint8_t* get_screen_buffer() const { return screen_front; }
    const uint8_t* get_vram_buffer() const { return reinterpret_cast<const uint8_t*>(vram_front); }
    const uint8_t* get_tilemap_buffer() const { return vram_front->tile_map_1; }
    
    // Swap buffers - call this once per frame from emulation thread
    void swap_buffers();

    // VRAM accessors (no mutex needed - writing to back buffer)
    uint8_t vram_read(uint16_t address);
    void vram_write(uint16_t address, uint8_t value);

    // OAM accessors
    uint8_t oam_read(uint16_t address);
    void oam_write(uint16_t address, uint8_t value);

private:
    // Double-buffered Video RAM (0x8000-0x9FFF) - using memcpy
    vram_layout vram_buffers[2];
    vram_layout* vram_back = &vram_buffers[0];  // Emulation thread writes here
    vram_layout* vram_front = &vram_buffers[1]; // Rendering thread reads from here
    
    // Double-buffered screen buffer - using memcpy
    uint8_t screen_buffers[2][SCREEN_BUFFER_SIZE] = {};
    uint8_t* screen_back = screen_buffers[0];  // Emulation thread writes here
    uint8_t* screen_front = screen_buffers[1]; // Rendering thread reads from here

    scanline_state_t sst = {};
    void handle_oam_search();
    void handle_pixel_transfer();
    void handle_hblank();
    void handle_vblank();
    void render_background(int i);
    void render_window(int i);
    void set_pixel(int x, int y, int scx, int scy); // Set a pixel in the screen buffer, scx and scy are 0 when used for window rendering
};