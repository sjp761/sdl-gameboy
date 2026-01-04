#pragma once
#include <cstdint>
#include <mutex>
#include <vector>
#include "ppu_constants.h"

class Bus;
class Cpu;
class LCD;


struct oam_entry
{
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    struct attributes
    {
        uint8_t unused : 4;       // Bits 0-3
        uint8_t palette_number : 1; // Bit 4
        uint8_t x_flip : 1;       // Bit 5
        uint8_t y_flip : 1;       // Bit 6
        uint8_t priority : 1;      // Bit 7
    } attr;
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
    std::vector<int> overlap_sprite_indices; 
    bool background_enabled; //Based on LCDC bit 0
    bool objs_enabled;       //Based on LCDC bit 1
    bool obj_size;          //Based on LCDC bit 2, false = 8x8, true = 8x16
};

struct scanline_context
{
    uint8_t* vram_base_ptr;
    uint8_t* bg_map_base_ptr;
    uint8_t* win_map_base_ptr;
    uint16_t tile_data_base_addr;
    bool     window_enabled;
    int      wx_start;
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
    
    // Get mutex for locking during VRAM access from rendering thread
    std::mutex& get_vram_mutex() { return vram_mutex; }
    std::mutex& get_screen_mutex() { return screen_mutex; }

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
    uint8_t screen_buffers[2][PpuConstants::SCREEN_BUFFER_SIZE] = {};
    uint8_t* screen_back = screen_buffers[0];  // Emulation thread writes here
    uint8_t* screen_front = screen_buffers[1]; // Rendering thread reads from here
    
    uint8_t bgwin_color_ids[PpuConstants::SCREEN_BUFFER_SIZE] = {}; // Using this to track raw BG color IDs for sprite priority handling, 

    // Mutex for thread-safe VRAM access from rendering thread
    mutable std::mutex vram_mutex;
    mutable std::mutex screen_mutex;

    scanline_context sctx = {}; // Used for per-scanline constants (pointers/ints/bools) that are helper values for pixel rendering (not internal GB state)
    scanline_state_t sst = {}; // Used for internal gameboy values (LY/SCX/SCY/WX/WY/etc)
    void handle_oam_search();
    void handle_pixel_transfer();
    void handle_hblank();
    void handle_vblank();
    void set_pixel(int x, int y, int offx, int offy, bool is_window, const scanline_context& ctx);
    void oam_render_scanline(const scanline_context& ctx);
};