#include <iostream>
#include <fstream>
#include <filesystem>
#include "cpu.h"
#include "instruction.h"
#include <json/json.h>
#include <json/value.h>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2) //Enforce a parameter
    {
        std::cerr << "Usage: " << argv[0] << " <opcode in hex>" << std::endl;
        return 1;
    }
    // Get the current working directory
    std::string cwd = std::filesystem::current_path();
    std::string jsonfile = cwd + "/tests/SSTs/v1/" + std::string(argv[1]) + ".json";
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
    Cpu cpu;
    
    cpu.opcode.whole = std::stoul(argv[1], nullptr, 16);
    cpu.instruction = Instruction(cpu.opcode);
}