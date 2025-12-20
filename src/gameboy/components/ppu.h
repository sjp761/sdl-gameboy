#pragma once
#include <cstdint>
class Bus;
class Cpu;
class LCD;
constexpr int SCANLINES_PER_FRAME = 154;
constexpr int VISIBLE_SCANLINES = 144;
constexpr int DOTS_PER_SCANLINE = 456;
constexpr int OAM_SEARCH_DOTS = 80;
constexpr int PIXEL_TRANSFER_DOTS = 172;
constexpr int HBLANK_DOTS = 204;

struct oam_entry
{
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attributes;
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
        uint8_t screen[160][144] = {}; //Pixel data for the screen
    private:
        void handle_oam_search();
        void handle_pixel_transfer();
        void handle_hblank();
        void handle_vblank();
};