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
    // Lock mutex to prevent rendering thread from reading during swap
    // Copy both to front buffer (better than pointer swap because rendering thread may be mid-read on one buffer)
    std::lock_guard<std::mutex> screen_lock(screen_mutex);
    std::memcpy(screen_front, screen_back, PpuConstants::SCREEN_BUFFER_SIZE);
    #ifdef ENABLE_DEBUG_VIEWERS
        std::lock_guard<std::mutex> vram_lock(vram_mutex);
        std::memcpy(vram_front, vram_back, sizeof(vram_layout));
    #endif
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
    if (dot >= PpuConstants::OAM_SEARCH_DOTS)
    {
        // Snapshot scroll and LY at the exact moment we enter pixel transfer
        sst.scy = lcd->regs.scroll_y;
        sst.scx = lcd->regs.scroll_x;
        sst.ly = lcd->regs.lcd_y;
        sst.wx = lcd->regs.window_x;
        sst.wy = lcd->regs.window_y;
        sst.background_enabled = lcd->get_lcd_control_attr(lcd_control_bits::BG_DISPLAY);
        sst.objs_enabled = lcd->get_lcd_control_attr(lcd_control_bits::OBJ_DISPLAY_ENABLE); //Can be toggled mid scanline but we will test it here for now
        sst.obj_size = lcd->get_lcd_control_attr(lcd_control_bits::OBJ_SIZE);
        sst.overlap_sprite_indices.clear();
        for (int i = 0; i < 40; i++)
        {
            int16_t sprite_y = static_cast<int16_t>(oam[i].y_pos) - 16; //Sprite Y position is offset by 16
            uint8_t sprite_height = sst.obj_size ? 16 : 8;
            if (sst.ly >= sprite_y && sst.ly < (sprite_y + sprite_height))
            {
                sst.overlap_sprite_indices.push_back(i);
                if (sst.overlap_sprite_indices.size() >= 10) //Max 10 sprites per scanline
                    break;
            }
        }
        lcd->set_mode(LCD_Modes::PIXEL_TRANSFER);
    }
}

void Ppu::handle_pixel_transfer() // We handle background drawing and window drawing here
{
    if (dot >= PpuConstants::OAM_SEARCH_DOTS + PpuConstants::PIXEL_TRANSFER_DOTS)
    {
        bool window_rendered_this_line = false;
        lcd->set_mode(LCD_Modes::HBLANK);

        for (int i = 0; i < 160; i++)
        {
            if (sst.wy <= sst.ly && i >= (static_cast<int>(sst.wx) - 7) && lcd->get_lcd_control_attr(lcd_control_bits::WINDOW_DISPLAY_ENABLE))
            {
                set_pixel(i, sst.window_line_counter, -(sst.wx - 7), 0, true); // Render window pixel
                window_rendered_this_line = true;
            }
            else
            {
                set_pixel(i, sst.ly, sst.scx, sst.scy, false); // Render background pixel
            }
        }
        if (sst.objs_enabled)
            oam_render_scanline(); // Render sprites on top of background/window
        if (window_rendered_this_line)
        {
            sst.window_line_counter++;
        }
    }
}

void Ppu::handle_hblank()
{
    if (static_cast<uint16_t>(dot) >= PpuConstants::DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();     
        if (lcd->regs.lcd_y >= PpuConstants::VISIBLE_SCANLINES)
        {
            lcd->set_mode(LCD_Modes::VBLANK);
            cpu->request_interrupt(Interrupts::InterruptMask::IT_VBlank);
        }
        else
        {
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
        sst.overlap_sprite_indices.clear(); //Clear sprite indices for next scanline
    }
}

void Ppu::handle_vblank()
{
    if (static_cast<uint16_t>(dot) >= PpuConstants::DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();        
        if (lcd->regs.lcd_y >= PpuConstants::SCANLINES_PER_FRAME)
        {
            sst.window_line_counter = 0; // Reset window line counter at the start of a new frame
            lcd->regs.lcd_y = 0;
            lcd->check_lyc();
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
    }
}


void Ppu::set_pixel(int x, int y, int offx, int offy, bool is_window) //When it is a window, offx is -(wx - 7), offy is 0 due to internal line counter
{
    uint8_t ly = sst.ly;     // LY is always used for setting the pixel y position so don't use the y parameter
    if (!sst.background_enabled)
    {
        if (!is_window)
        {
            bgwin_color_ids[ly * PpuConstants::SCREEN_WIDTH + x] = 0; // Update the bg color ID tracking array for priority handling
            screen_back[ly * PpuConstants::SCREEN_WIDTH + x] = lcd->regs.bg_palette.get_color(0); // Write the color ID to the screen buffer
        }
        return;
    }
    uint16_t tile_map_base_addr;
    if (is_window)
        tile_map_base_addr = lcd->get_lcd_control_attr(lcd_control_bits::WINDOW_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800; //Which tile map to use for window
    else
        tile_map_base_addr = lcd->get_lcd_control_attr(lcd_control_bits::BG_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800; //Which tile map to use for background
    uint16_t tile_data_base_addr = lcd->get_lcd_control_attr(lcd_control_bits::BG_WINDOW_TILE_DATA_SELECT) ? 0x8000 : 0x9000; // Bit 4 determines adressing mode, 8000 or 9000 is the base addr, 8000 uses unsigned math while 9000 uses signed math
    uint8_t* vram_base_ptr = reinterpret_cast<uint8_t*>(vram_back); // Base pointer to VRAM back buffer
    uint8_t* tile_map_base_ptr = vram_base_ptr + (tile_map_base_addr - 0x8000); // Pointer to the start of the selected tile map
    uint8_t tile_row = ((y + offy) & 255) / 8; 
    uint8_t tile_col = ((x + offx) & 255) / 8; 
    uint8_t tile_index = tile_map_base_ptr[tile_row * PpuConstants::TILE_MAP_WIDTH + tile_col]; // Get the tile index from the tile map
    uint16_t tile_data_addr;
    if (tile_data_base_addr == 0x8000)
        tile_data_addr = 0x8000 + (tile_index * 16); // Each tile is 16 bytes, tile indices always range from 0-255
    else
        tile_data_addr = 0x9000 + (static_cast<int8_t>(tile_index) * 16); // Signed index for 9000 addressing mode, casting uint8_t might be risky but worth a shot
    uint8_t line_within_tile = (y + offy) % 8; // Which line within the tile we are drawing (0-7), scy is needed here otherwise text will appear garbled 
    uint8_t pixel_within_tile = (x + offx) % 8; // Which pixel within the tile we are drawing (0-7), since SCX and SCY can be between 0 and 255 (not 32) the gameboy can render tiles partially (like half a tile on the left side of the screen)
    uint8_t byte1 = vram_base_ptr[tile_data_addr - 0x8000 + (line_within_tile * 2)];     // Each line is 2 bytes so multiply by 2
    uint8_t byte2 = vram_base_ptr[tile_data_addr - 0x8000 + (line_within_tile * 2) + 1]; // Second byte
    uint8_t bit_index = 7 - pixel_within_tile; // Bits are ordered from MSB to LSB, so we need to invert the pixel index
    uint8_t color_bit0 = (byte1 >> bit_index) & 0x01; // Get the bit for color 0
    uint8_t color_bit1 = (byte2 >> bit_index) & 0x01; // Get the bit for color 1
    uint8_t color_id = (color_bit1 << 1) | color_bit0;
    bgwin_color_ids[ly * PpuConstants::SCREEN_WIDTH + x] = color_id; // Update the bg color ID tracking array for priority handling
    screen_back[ly * PpuConstants::SCREEN_WIDTH + x] = lcd->regs.bg_palette.get_color(color_id); // Write the color ID to the screen buffer
}

void Ppu::oam_render_scanline()
{
    for (int sprite_idx : sst.overlap_sprite_indices)
    {
        const oam_entry& sprite = oam[sprite_idx];
        int16_t sprite_y = static_cast<int16_t>(sprite.y_pos) - 16; // Sprite Y position is offset by 16
        int16_t sprite_x = static_cast<int16_t>(sprite.x_pos) - 8;  // Sprite X position is offset by 8
        uint8_t sprite_height = sst.obj_size ? 16 : 8;
        uint8_t line_within_sprite = sst.ly - sprite_y;
        if (sprite.attr.y_flip)
            line_within_sprite = (sprite_height - 1) - line_within_sprite; // Flip vertically if needed, get maximum line index then subtract current line to "flip" the number
        uint8_t tile_index = sprite.tile_index;
        if (sst.obj_size)
        {
            tile_index &= 0xFE; // 8x16 sprites always start on an even-numbered tile (ignore bit 0)
            if (line_within_sprite >= 8)
                tile_index += 1; // Bottom half uses the following tile in the pair
        }
        uint8_t* vram_base_ptr = reinterpret_cast<uint8_t*>(vram_back); // Base pointer to VRAM back buffer, OBJs use 0x8000 addressing mode only, all tiles reside in order, 8x16 uses two consecutive tiles
        uint8_t* tile_base_ptr = vram_base_ptr + tile_index * 16; // Pointer to the start of the tile data for this sprite
        uint8_t byte1 = tile_base_ptr[(line_within_sprite & 7) * 2];     // Each line is 2 bytes so multiply by 2
        uint8_t byte2 = tile_base_ptr[(line_within_sprite & 7) * 2 + 1]; // Second byte
        for (int bit = 7; bit >= 0; bit--)
        {
            int screen_x_offset = sprite.attr.x_flip ? bit : (7 - bit);
            int screen_x = sprite_x + screen_x_offset; // Calculate screen X position
            if (screen_x < 0 || screen_x >= PpuConstants::SCREEN_WIDTH)
                continue; // Skip pixels outside the screen
            uint8_t color_bit0 = (byte1 >> bit) & 0x01; // Get the bit for color 0
            uint8_t color_bit1 = (byte2 >> bit) & 0x01; // Get the bit for color 1
            uint8_t color_id = (color_bit1 << 1) | color_bit0;
            if (color_id == 0)
                continue; // Color ID 0 is transparent for sprites
            if (sprite.attr.priority && bgwin_color_ids[sst.ly * PpuConstants::SCREEN_WIDTH + screen_x] != 0) 
                continue; // Skip drawing this pixel due to priority, if priority bit is set, color ID 1-3 of BG/WIN have priority over sprite
            pallette_data& obj_palette = sprite.attr.palette_number ? lcd->regs.obj_palette_1 : lcd->regs.obj_palette_0;
            screen_back[sst.ly * PpuConstants::SCREEN_WIDTH + screen_x] = obj_palette.get_color(color_id); // Write the color ID to the screen buffer
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