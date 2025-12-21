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
    // Initialize all VRAM to 0
    std::memset(&vram, 0, sizeof(vram));
}

void Ppu::copy_screen_buffer(uint8_t* dest) const
{
    std::lock_guard<std::mutex> lock(screen_mutex); // Lock the screen buffer during copy
    std::memcpy(dest, screen, SCREEN_BUFFER_SIZE); // Quick copy under lock to minimize contention between PPU writes and SDL reads
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
        lcd->set_mode(LCD_Modes::PIXEL_TRANSFER);
    }
}

void Ppu::handle_pixel_transfer() //We handle background drawing and window drawing here
{
    // Only draw the scanline once when we first enter pixel transfer mode (at dot 80)
    if (dot == OAM_SEARCH_DOTS)
    {
        uint8_t scy = lcd->regs.scroll_y;
        uint8_t scx = lcd->regs.scroll_x;
        uint8_t ly = lcd->regs.lcd_y;

        // TODO: Implement actual pixel drawing here
        // When drawing is implemented, wrap screen writes with std::lock_guard<std::mutex> lock(screen_mutex);
    }
    if (dot >= OAM_SEARCH_DOTS + PIXEL_TRANSFER_DOTS)
    {
        lcd->set_mode(LCD_Modes::HBLANK);
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
    std::lock_guard<std::mutex> lock(vram_mutex);
    uint16_t offset = address - 0x8000;
    return reinterpret_cast<uint8_t*>(&vram)[offset];
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
    std::lock_guard<std::mutex> lock(vram_mutex);
    uint16_t offset = address - 0x8000;
    reinterpret_cast<uint8_t*>(&vram)[offset] = value;
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