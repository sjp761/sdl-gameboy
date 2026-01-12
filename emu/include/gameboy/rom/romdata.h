#pragma once
#include <string>
#include <memory>
#include "romhelpers.h"

struct RomData // This will hold the rom data before being loaded into the Rom class
{
    std::unique_ptr<uint8_t[]> rom_bytes;
    cart_context ctx;
    RomData(const std::string &filename, const std::string& bootrom_filename);
    std::string cart_type_name();
    std::string cart_lic_name();
    void load_rom(const std::string &filename);
    void load_bootrom(const std::string &bootrom_filename);
    
private:
    bool read_file_data(const std::string &filepath, uint8_t* buffer, size_t expected_size, const std::string &file_type);
};