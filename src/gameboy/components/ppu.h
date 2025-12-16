#pragma once
#include <cstdint>
class Bus;

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
        oam_entry oam[40] = {};
        Ppu();
        void set_cmp(Bus* bus_ptr);
        Bus* bus;
};