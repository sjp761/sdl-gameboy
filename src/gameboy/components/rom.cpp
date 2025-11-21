#include "rom.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include "emu.h"

bool Rom::cart_load(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return false;
    }

    ctx.rom_size = static_cast<uint32_t>(file.tellg());
    ctx.rom_data = std::make_unique<uint8_t[]>(ctx.rom_size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(ctx.rom_data.get()), ctx.rom_size);

    // Import ROM header (first 0x50 bytes from 0x0100 to 0x014F)

    std::memcpy(ctx.header.entry, &ctx.rom_data[0x100], 4);                // 4 bytes
    std::memcpy(ctx.header.logo, &ctx.rom_data[0x104], 0x30);              // 48 bytes
    std::memcpy(ctx.header.title, &ctx.rom_data[0x134], 16);               // 16 bytes
    ctx.header.new_lic_code    = static_cast<uint16_t>(ctx.rom_data[0x144] | (ctx.rom_data[0x145] << 8)); // 2 bytes
    ctx.header.sgb_flag        = ctx.rom_data[0x146];                      // 1 byte
    ctx.header.type            = ctx.rom_data[0x147];                      // 1 byte
    ctx.header.rom_size        = ctx.rom_data[0x148];                      // 1 byte
    ctx.header.ram_size        = ctx.rom_data[0x149];                      // 1 byte
    ctx.header.dest_code       = ctx.rom_data[0x14A];                      // 1 byte
    ctx.header.lic_code        = ctx.rom_data[0x14B];                      // 1 byte
    ctx.header.version         = ctx.rom_data[0x14C];                      // 1 byte
    ctx.header.checksum        = ctx.rom_data[0x14D];                      // 1 byte
    ctx.header.global_checksum = static_cast<uint16_t>(ctx.rom_data[0x14E] | (ctx.rom_data[0x14F] << 8)); // 2 bytes

    std::cout << "Cartridge Loaded:\n";
    std::cout << "\t Title    : " << ctx.header.title << "\n";
    std::cout << "\t Type     : " << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                << static_cast<int>(ctx.header.type) << " (" << cart_type_name() << ")\n";
    std::cout << "\t ROM Size : " << (32 << ctx.header.rom_size) << " KB\n";
    std::cout << "\t RAM Size : " << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(ctx.header.ram_size) << "\n";
    std::cout << "\t LIC Code : " << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(ctx.header.lic_code) << " (" << cart_lic_name() << ")\n";
    std::cout << "\t ROM Vers : " << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(ctx.header.version) << std::dec << "\n";
    uint16_t x = 0;

    for (uint16_t i=0x0134; i<=0x014C; i++) 
    {
        x = x - ctx.rom_data[i] - 1;
    }

    printf("\t Checksum : %2.2X (%s)\n", ctx.header.checksum, (x & 0xFF) ? "PASSED" : "FAILED");

    return true;
}

std::string Rom::cart_lic_name()
{
    auto it = LIC_CODE.find(ctx.header.lic_code);
    if (it != LIC_CODE.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";
}

std::string Rom::cart_type_name()
{
    auto it = ROM_TYPES.find(ctx.header.type);
    if (it != ROM_TYPES.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";}

uint8_t Rom::cart_read(uint16_t address)
{
    return Emu::cmp.rom.ctx.rom_data[address];
}

void Rom::cart_write(uint16_t address, uint8_t data)
{
    // Writing to ROM is typically not allowed for read-only memory.
    // However, if this is a writable cartridge (e.g., for save data), handle it here.
    if (address < ctx.rom_size) {
        ctx.rom_data[address] = data;
    } else {
        std::cerr << "Write attempt out of ROM bounds at address: 0x" 
                  << std::hex << std::uppercase << address << std::dec << std::endl;
    }
}

void Rom::create_blank_rom(uint32_t size)
{
    ctx.rom_size = size;
    ctx.rom_data = std::make_unique<uint8_t[]>(ctx.rom_size);
    std::memset(ctx.rom_data.get(), 0xFF, ctx.rom_size); // Initialize with 0xFF (common for blank ROMs)

    // Clear the ROM header
    std::memset(&ctx.header, 0, sizeof(ctx.header));
    std::cout << "Blank ROM of size " << ctx.rom_size << " bytes created.\n";
}
