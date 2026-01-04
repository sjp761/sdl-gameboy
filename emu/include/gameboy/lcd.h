#pragma once
#include <cstdint>
#include "ppu.h"
#include <unordered_map>

struct pallette_data 
{
    uint8_t color_0 : 2; // Bits 0-1
    uint8_t color_1 : 2; // Bits 2-3
    uint8_t color_2 : 2; // Bits 4-5
    uint8_t color_3 : 2; // Bits 6-7

    uint8_t get_color(uint8_t index) const 
    {
        switch(index)
        {
            case 0: return color_0;
            case 1: return color_1;
            case 2: return color_2;
            case 3: return color_3;
            default: return 0; // Invalid index
        }
    }

    pallette_data(uint8_t value = 0) // Allows implicit conversion from uint8_t
    {
        color_0 = value & 0b00000011;
        color_1 = (value >> 2) & 0b00000011;
        color_2 = (value >> 4) & 0b00000011;
        color_3 = (value >> 6) & 0b00000011;
    }

    operator uint8_t() const // User defined conversion to uint8_t
    {
        return (color_0) | (color_1 << 2) | (color_2 << 4) | (color_3 << 6);
    }
};

// LCD Control Register (FF40) bit-field struct
struct lcd_control_register
{
    uint8_t bg_display : 1;                    // Bit 0: BG Display
    uint8_t obj_display_enable : 1;            // Bit 1: OBJ Display Enable
    uint8_t obj_size : 1;                      // Bit 2: OBJ Size (0=8x8, 1=8x16)
    uint8_t bg_tile_map_display_select : 1;    // Bit 3: BG Tile Map Display Select
    uint8_t bg_window_tile_data_select : 1;    // Bit 4: BG & Window Tile Data Select
    uint8_t window_display_enable : 1;         // Bit 5: Window Display Enable
    uint8_t window_tile_map_display_select : 1;// Bit 6: Window Tile Map Display Select
    uint8_t lcd_enable : 1;                    // Bit 7: LCD Enable
    
    // Constructor to set default values (0b10010001)
    lcd_control_register() : bg_display(1), obj_display_enable(0), obj_size(0),
                             bg_tile_map_display_select(0), bg_window_tile_data_select(1),
                             window_display_enable(0), window_tile_map_display_select(0),
                             lcd_enable(1) {}
};

enum class lcd_control_bits : uint8_t
{
    BG_DISPLAY = 0,
    OBJ_DISPLAY_ENABLE = 1,
    OBJ_SIZE = 2,
    BG_TILE_MAP_DISPLAY_SELECT = 3,
    BG_WINDOW_TILE_DATA_SELECT = 4,
    WINDOW_DISPLAY_ENABLE = 5,
    WINDOW_TILE_MAP_DISPLAY_SELECT = 6,
    LCD_ENABLE = 7
};

// LCD Status Register (FF41) bit-field struct
struct lcd_status_register
{
    uint8_t mode_flag : 2;              // Bits 0-1: Mode Flag
    uint8_t lyc_eq_ly_flag : 1;         // Bit 2: LYC=LY Flag
    uint8_t mode_0_hblank_interrupt : 1;// Bit 3: Mode 0 HBlank Interrupt
    uint8_t mode_1_vblank_interrupt : 1;// Bit 4: Mode 1 VBlank Interrupt
    uint8_t mode_2_oam_interrupt : 1;   // Bit 5: Mode 2 OAM Interrupt
    uint8_t lyc_eq_ly_interrupt : 1;    // Bit 6: LYC=LY Interrupt
    uint8_t unused : 1;                 // Bit 7: Unused
    
    // Constructor to set default values (0b00000001 - mode 1 VBLANK for bootrom compatibility)
    lcd_status_register() : mode_flag(1), lyc_eq_ly_flag(0), mode_0_hblank_interrupt(0),
                           mode_1_vblank_interrupt(0), mode_2_oam_interrupt(0),
                           lyc_eq_ly_interrupt(0), unused(0) {}
};

enum class lcd_status_bits : uint8_t
{
    MODE_FLAG = 0,
    LYC_EQ_LY_FLAG = 2,
    MODE_0_HBLANK_INTERRUPT = 3,
    MODE_1_VBLANK_INTERRUPT = 4,
    MODE_2_OAM_INTERRUPT = 5,
    LYC_EQ_LY_INTERRUPT = 6
};

struct lcd_registers
{
    //Gameboy specific LCD registers
    lcd_control_register lcd_control; //FF40
    lcd_status_register lcd_status;   //FF41
    uint8_t scroll_y;    //FF42
    uint8_t scroll_x;    //FF43
    uint8_t lcd_y;       //FF44
    uint8_t lcd_y_compare; //FF45
    //FF46 is DMA transfer register, handled directly in bus
    pallette_data bg_palette;  //FF47
    pallette_data obj_palette_0; //FF48
    pallette_data obj_palette_1; //FF49
    uint8_t window_y;    //FF4A
    uint8_t window_x;    //FF4B
    
};

// Helper functions to convert between bit-field structs and uint8_t
inline uint8_t lcd_control_to_byte(const lcd_control_register& ctrl)
{
    return *reinterpret_cast<const uint8_t*>(&ctrl); // Casts address to a pointer then dereferences it
}

inline uint8_t lcd_status_to_byte(const lcd_status_register& status)
{
    return *reinterpret_cast<const uint8_t*>(&status);
}

inline void byte_to_lcd_control(lcd_control_register& ctrl, uint8_t value)
{
    *reinterpret_cast<uint8_t*>(&ctrl) = value;
}

inline void byte_to_lcd_status(lcd_status_register& status, uint8_t value)
{
    // Preserve lower 2 bits (mode_flag) as they're read-only
    uint8_t mode = status.mode_flag;
    *reinterpret_cast<uint8_t*>(&status) = value;
    status.mode_flag = mode;
}

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
        uint8_t get_lcd_control_attr(lcd_control_bits bit);
        uint8_t get_lcd_status_attr(lcd_status_bits bit);
        void check_lyc();

};