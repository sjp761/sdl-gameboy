#include "emu.h"
#include "cpu/cpu_tables.h"
#include <filesystem>
#include <iostream>

#include "romdata.h"
// Constructor initializes pointers to nullptr
Emu::Emu(const std::string &rom_filename, const std::string &bootrom_filename) 
    : bus(),
      cpu(),
      timer(),
      dma(),
      lcd()
{
    ctx.paused = false;
    ctx.running = true;
    ctx.ticks = 0;
    rom = create_cartridge(rom_filename, bootrom_filename);
}

Emu::Emu(bool test_mode_enable)
    : bus(test_mode_enable),
      cpu(),
      timer(),
      dma(),
      lcd()
{
    ctx.paused = false;
    ctx.running = true;
    ctx.ticks = 0;
    //  Will create a dummy ROM for test mode
}

ROM* Emu::create_cartridge(const std::string &filename, const std::string& bootrom_filename)
{
    namespace fs = std::filesystem;
    RomData rom_data = RomData(filename, bootrom_filename); // Temporary RomData to load the ROM file
    if (!rom_data.ctx.rom_loaded)
    {
        std::cout << "ROM did not load, using blank ROM with bootrom only\n";
        return new ROM(rom_data); // Return blank ROM if loading failed
    }
    uint8_t rom_type = rom_data.ctx.header.type;
    ROM* romptr = nullptr;
    switch (rom_type)
    {
        case 0x00: // ROM ONLY
            romptr = new ROM(rom_data); // std::move called in constructor
            break;
        // Future MBC types will go here
        default:
            std::cerr << "Unsupported or unimplemented ROM type: " << std::hex << static_cast<int>(rom_type) << std::dec << "\n";
            romptr = new ROM(rom_data); // Fallback to ROM ONLY for now
            break;
    }
    return romptr;
}

void Emu::set_component_pointers()
{
  cpu.set_cmp(&bus, &timer, &dma, &ppu);
  bus.set_cmp(rom, &timer, &ppu, &dma, &lcd);
  timer.set_cmp(&bus);
  ppu.set_cmp(&bus, &lcd, &cpu);
  dma.set_cmp(&bus);
  lcd.set_cmp(&ppu, &cpu);
}
