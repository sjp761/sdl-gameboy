#include "romdata.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstring>
    
RomData::RomData(const std::string &filename, const std::string &bootrom_filename)
{
    load_bootrom(bootrom_filename);
    load_rom(filename);
}

std::string RomData::cart_lic_name()
{
    auto it = LIC_CODE.find(ctx.header.lic_code);
    if (it != LIC_CODE.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";
}

void RomData::load_rom(const std::string &filename)
{
    namespace fs = std::filesystem;
    
    if (filename.empty()) 
    {
        std::cout << "No ROM file provided, starting with bootrom only.\n";
        ctx.rom_loaded = false;
        return;
    }

    if (!fs::exists(filename)) 
    {
        std::cerr << "ROM file does not exist: " << filename << std::endl;
        ctx.rom_loaded = false;
        return;
    }

    if (!fs::is_regular_file(filename)) 
    {
        std::cerr << "Path is not a regular file: " << filename << std::endl;
        ctx.rom_loaded = false;
        return;
    }

    ctx.rom_size = static_cast<uint32_t>(fs::file_size(filename));
    ctx.rom_data = std::make_unique<uint8_t[]>(ctx.rom_size);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        ctx.rom_loaded = false;
        return;
    }

    file.read(reinterpret_cast<char*>(ctx.rom_data.get()), ctx.rom_size);
    if (!file) {
        std::cerr << "Failed to read ROM file: " << filename << std::endl;
        ctx.rom_loaded = false;
        return;
    }

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
    std::cout << "\t ROM Size : " << cart_rom_size_name() << "\n";
    std::cout << "\t RAM Size : " << cart_ram_size_name() << "\n";
    std::cout << "\t LIC Code : " << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(ctx.header.lic_code) << " (" << cart_lic_name() << ")\n";
    std::cout << "\t ROM Vers : " << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(ctx.header.version) << std::dec << "\n";

    uint16_t x = 0;
    for (uint16_t i = 0x0134; i <= 0x014C; i++) {
        x = x - ctx.rom_data[i] - 1;
    }

    std::cout << "\t Checksum : " << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(ctx.header.checksum) << " (" << ((x & 0xFF) ? "PASSED" : "FAILED") << ")\n";
    
    ctx.rom_loaded = true;
}

void RomData::load_bootrom(const std::string &bootrom_filename)
{
    namespace fs = std::filesystem;
    
    if (!fs::exists(bootrom_filename)) {
        std::cerr << "Bootrom file does not exist: " << bootrom_filename << std::endl;
        return;
    }
    
    auto size = fs::file_size(bootrom_filename);
    if (size != 0x100) {
        std::cerr << "Invalid bootrom size (expected 256 bytes): " << size << std::endl;
        return;
    }
    
    std::ifstream bootrom_file(bootrom_filename, std::ios::binary);
    if (!bootrom_file) {
        std::cerr << "Failed to open bootrom file: " << bootrom_filename << std::endl;
        return;
    }
    
    bootrom_file.read(reinterpret_cast<char*>(ctx.bootrom_data), 0x100);
    if (!bootrom_file) {
        std::cerr << "Failed to read bootrom file: " << bootrom_filename << std::endl;
        return;
    }
    
    ctx.bootrom_enabled = true;
    std::cout << "Bootrom loaded successfully\n";
}

std::string RomData::cart_type_name()
{
    auto it = ROM_TYPES.find(ctx.header.type);
    if (it != ROM_TYPES.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";
}

std::string RomData::cart_rom_size_name()
{
    auto it = ROM_SIZES.find(ctx.header.rom_size);
    if (it != ROM_SIZES.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";
}

std::string RomData::cart_ram_size_name()
{
    auto it = RAM_SIZES.find(ctx.header.ram_size);
    if (it != RAM_SIZES.end()) 
    {
        return it->second;
    }
    return "UNKNOWN";
}

