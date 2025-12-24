#include "ppu.h"
#include <cstdint>
#include <cstring>
#include "lcd.h"
#include "interrupts.h"
#include "cpu.h"
#include <mutex>
class LCD;

Ppu::Ppu() : bus(nullptr), lcd(nullptr), cpu(nullptr), dot(0)
{
    // Initialize both VRAM buffers to 0
    std::memset(&vram_buffers[0], 0, sizeof(vram_layout));
    std::memset(&vram_buffers[1], 0, sizeof(vram_layout));
    // Screen buffers are already initialized to 0 by their declarations
}

void Ppu::swap_buffers()
{
    // Copy both to front buffer (better than pointer swap because rendering thread may be mid-read on one buffer)
    std::memcpy(screen_front, screen_back, SCREEN_BUFFER_SIZE);
    std::memcpy(vram_front, vram_back, sizeof(vram_layout));
}

void Ppu::set_cmp(Bus *bus_ptr, LCD* lcd_ptr, Cpu* cpu_ptr)
{
    bus = bus_ptr;
    lcd = lcd_ptr;
    cpu = cpu_ptr;
}

void Ppu::ppu_tick()
{
    dot++;
    switch (static_cast<LCD_Modes>(lcd->regs.lcd_status.mode_flag))
    {
        case LCD_Modes::OAM_SEARCH:
            handle_oam_search();
            break;
        case LCD_Modes::PIXEL_TRANSFER:
            handle_pixel_transfer();
            break;
        case LCD_Modes::HBLANK:
            handle_hblank();
            break;
        case LCD_Modes::VBLANK:
            handle_vblank();
            break;
    }
}

void Ppu::handle_oam_search()
{
    if (dot >= OAM_SEARCH_DOTS)
    {
        // Snapshot scroll and LY at the exact moment we enter pixel transfer
        sst.scy = lcd->regs.scroll_y;
        sst.scx = lcd->regs.scroll_x;
        sst.ly = lcd->regs.lcd_y;
        lcd->set_mode(LCD_Modes::PIXEL_TRANSFER);
    }
}

void Ppu::handle_pixel_transfer() // We handle background drawing and window drawing here
{
    if (dot >= OAM_SEARCH_DOTS + PIXEL_TRANSFER_DOTS)
    {
        lcd->set_mode(LCD_Modes::HBLANK);
        for (int i = 0; i < 160; i++) //i is the pixel x position in the scanline
        {
            uint16_t tile_map_base_addr = lcd->regs.lcd_control.bg_tile_map_display_select ? 0x9C00 : 0x9800; //Which tile map to use for background
            uint16_t tile_data_base_addr = lcd->regs.lcd_control.bg_window_tile_data_select ? 0x8000 : 0x9000; // Bit 4 determines adressing mode, 8000 or 9000 is the base addr, 8000 uses unsigned math while 9000 uses signed math
            uint8_t scy = sst.scy; // Scroll Y, starting Y position in the background
            uint8_t scx = sst.scx; // Scroll X, starting X position in the background
            uint8_t ly = sst.ly;     // Current scanline (LY)
            uint8_t* vram_base_ptr = reinterpret_cast<uint8_t*>(vram_back); // Base pointer to VRAM back buffer
            uint8_t* tile_map_base_ptr = vram_base_ptr + (tile_map_base_addr - 0x8000); // Pointer to the start of the selected tile map

            uint8_t tile_row = ((scy + ly) & 255) / 8; // The row within the entire 32*32 high background (0-255), dividing by 8 (rather than modding by 32) makes a bit more sense here as it seems it will give the actual tile
            uint8_t tile_col = ((scx + i) & 255) / 8; // The column within the entire 32*32 wide background (0-255)
            uint8_t tile_index = tile_map_base_ptr[tile_row * TILE_MAP_WIDTH + tile_col]; // Get the tile index from the tile map
            // Calculate the address of the tile data
            uint16_t tile_data_addr;
            if (tile_data_base_addr == 0x8000)
                tile_data_addr = 0x8000 + (tile_index * 16); // Each tile is 16 bytes, tile indices always range from 0-255
            else
                tile_data_addr = 0x9000 + (static_cast<int8_t>(tile_index) * 16); // Signed index for 9000 addressing mode, casting uint8_t might be risky but worth a shot
            uint8_t line_within_tile = (scy + ly) % 8; // Which line within the tile we are drawing (0-7), scy is needed here otherwise text will appear garbled 
            uint8_t pixel_within_tile = (scx + i) % 8; // Which pixel within the tile we are drawing (0-7), since SCX and SCY can be between 0 and 255 (not 32) the gameboy can render tiles partially (like half a tile on the left side of the screen)
            uint8_t byte1 = vram_base_ptr[tile_data_addr - 0x8000 + (line_within_tile * 2)];     // Each line is 2 bytes so multiply by 2
            uint8_t byte2 = vram_base_ptr[tile_data_addr - 0x8000 + (line_within_tile * 2) + 1]; // Second byte
            uint8_t bit_index = 7 - pixel_within_tile; // Bits are ordered from MSB to LSB, so we need to invert the pixel index
            uint8_t color_bit0 = (byte1 >> bit_index) & 0x01; // Get the bit for color 0
            uint8_t color_bit1 = (byte2 >> bit_index) & 0x01; // Get the bit for color 1
            uint8_t color_id = (color_bit1 << 1) | color_bit0;
            screen_back[ly * SCREEN_WIDTH + i] = color_id; // Write the color ID to the screen buffer
        }

    }
}

void Ppu::handle_hblank()
{
    if (static_cast<uint16_t>(dot) >= DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();     
        if (lcd->regs.lcd_y >= VISIBLE_SCANLINES)
        {
            lcd->set_mode(LCD_Modes::VBLANK);
            cpu->request_interrupt(Interrupts::InterruptMask::IT_VBlank);
        }
        else
        {
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
    }
}

void Ppu::handle_vblank()
{
    if (static_cast<uint16_t>(dot) >= DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();        
        if (lcd->regs.lcd_y >= SCANLINES_PER_FRAME)
        {
            lcd->regs.lcd_y = 0;
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
    }
}

uint8_t Ppu::vram_read(uint16_t address)
{
    // If LCD if off, vram is always accessible
    // If LCD is on, vram is only accessible when not in mode 3 (PIXEL_TRANSFER)
    if (lcd && lcd->regs.lcd_control.lcd_enable) 
    {
        // If LCD is enabled, only allow reads outside of mode 3
        if (lcd->regs.lcd_status.mode_flag == static_cast<uint8_t>(LCD_Modes::PIXEL_TRANSFER)) {
            // VRAM is locked during pixel transfer - return 0xFF
            return 0xFF;
        }
    }
    
    // VRAM range: 0x8000-0x9FFF (8KB)
    uint16_t offset = address - 0x8000;
    return reinterpret_cast<uint8_t*>(&vram_back)[offset];
}

void Ppu::vram_write(uint16_t address, uint8_t value)
{
    // VRAM is only accessible when LCD is off or when not in mode 3 (PIXEL_TRANSFER)
    if (lcd && lcd->regs.lcd_control.lcd_enable) {
        // If LCD is enabled, only allow writes outside of mode 3
        if (lcd->regs.lcd_status.mode_flag == static_cast<uint8_t>(LCD_Modes::PIXEL_TRANSFER)) {
            // VRAM is locked during pixel transfer - ignore write
            return;
        }
    }
    
    // VRAM range: 0x8000-0x9FFF (8KB)
    uint16_t offset = address - 0x8000;
    reinterpret_cast<uint8_t*>(vram_back)[offset] = value;
}

uint8_t Ppu::oam_read(uint16_t address)
{
    // OAM range: 0xFE00-0xFE9F (160 bytes)
    uint8_t offset = address - 0xFE00;
    uint8_t* oam_ptr = reinterpret_cast<uint8_t*>(&oam);
    return oam_ptr[offset];
}

void Ppu::oam_write(uint16_t address, uint8_t value)
{
    // OAM range: 0xFE00-0xFE9F (160 bytes)
    uint8_t offset = address - 0xFE00;
    uint8_t* oam_ptr = reinterpret_cast<uint8_t*>(&oam);
    oam_ptr[offset] = value;
}