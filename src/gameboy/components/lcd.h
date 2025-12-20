#pragma once
#include <cstdint>
#include "ppu.h"
#include <unordered_map>
struct lcd_registers
{
    //Gameboy specific LCD registers
    uint8_t lcd_control = 0b10010001; //FF40, Set enable LCD, BG display, and BG tile data area by default
    uint8_t lcd_status = 0b00000011;  //FF41, Default mode is 3 (pixel transfer)
    uint8_t scroll_y;    //FF42
    uint8_t scroll_x;    //FF43
    uint8_t lcd_y;       //FF44
    uint8_t lcd_y_compare; //FF45
    //FF46 is DMA transfer register, handled directly in bus
    uint8_t bg_palette;  //FF47
    uint8_t obj_palette_0; //FF48
    uint8_t obj_palette_1; //FF49
    uint8_t window_y;    //FF4A
    uint8_t window_x;    //FF4B
    
};

enum class LCD_Colors : uint8_t
{
    WHITE = 0b00,
    LIGHT_GRAY = 0b01,
    DARK_GRAY = 0b10,
    BLACK = 0b11
};


inline std::unordered_map<LCD_Colors, uint32_t> SDL_LCD_color_codes = 
{
    {LCD_Colors::WHITE, 0xFFFFFFFF},
    {LCD_Colors::LIGHT_GRAY, 0xFFAAAAAA},
    {LCD_Colors::DARK_GRAY, 0xFF555555},
    {LCD_Colors::BLACK, 0xFF000000}
};

enum class LCD_Control_Flags : uint8_t
{
    LCD_ENABLE = 0b10000000, //Flags by masks
    WINDOW_TILE_MAP_DISPLAY_SELECT = 0b01000000,
    WINDOW_DISPLAY_ENABLE = 0b00100000,
    BG_WINDOW_TILE_DATA_SELECT = 0b00010000,
    BG_TILE_MAP_DISPLAY_SELECT = 0b00001000,
    OBJ_SIZE = 0b00000100,
    OBJ_DISPLAY_ENABLE = 0b00000010,
    BG_DISPLAY = 0b00000001
};

enum class LCD_Status_Flags : uint8_t
{
    LYC_EQ_LY_INTERRUPT = 0b01000000,
    MODE_2_OAM_INTERRUPT = 0b00100000,
    MODE_1_VBLANK_INTERRUPT = 0b00010000,
    MODE_0_HBLANK_INTERRUPT = 0b00001000,
    LYC_EQ_LY_FLAG = 0b00000100,
    MODE_FLAG = 0b00000011
};

enum class LCD_Modes : uint8_t //For bits 0-1 of LCD Status register
{
    HBLANK = 0,
    VBLANK = 1,
    OAM_SEARCH = 2,
    PIXEL_TRANSFER = 3
};

class LCD
{
    public:
        LCD();
        lcd_registers regs;
        Ppu* ppu;
        Cpu* cpu;
        void set_cmp(Ppu* ppu_ptr, Cpu* cpu_ptr);
        void lcd_write(uint16_t addr, uint8_t value);
        uint8_t lcd_read(uint16_t addr);
        void bump_ly();
        void set_mode(LCD_Modes mode);
};