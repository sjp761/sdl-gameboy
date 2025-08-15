#include <iostream>
#include <fstream>
#include <filesystem>
#include "cpu.h"
#include "emu.h"
#include "instruction.h"
#include <json/json.h>
#include <json/value.h>
#include <string>
#include <assert.h>

int main(int argc, char* argv[])
{
    std::string opcode_hex;
    if (argc < 2) //Enforce a parameter
    {
        opcode_hex = "01"; // Default opcode if none provided
    }
    else
    {
        opcode_hex = std::string(argv[1]);
    }
    // Get the current working directory
    std::string cwd = std::filesystem::current_path();
    std::string jsonfile = cwd + "/tests/SSTs/v1/" + opcode_hex + ".json";
    // Parse the given jsonfile
    Json::Value root;
    Json::Reader reader;
    std::ifstream file(jsonfile, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonfile << std::endl;
        return 1;
    }
    reader.parse(file, root);
    file.close();
    Emu emu;
    emu.cmp.cpu.opcode.whole = std::stoul(opcode_hex, nullptr, 16);
    emu.cmp.cpu.instruction = Instruction(emu.cmp.cpu.opcode);
    for (int i = 0; i < root.size(); ++i)
    {
        emu.cmp.cpu.set_opcode_test_data(root, i);
        emu.cmp.cpu.fetch_data();
        emu.cmp.cpu.execute_instruction();
        if (!emu.cmp.cpu.check_opcode_data(root, i))
        {
            std::cerr << "Opcode test failed for index: " << i << std::endl;
            return 1; // Exit with error if any test fails
        }
    }
}