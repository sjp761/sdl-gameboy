#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include "rom.h"
#include "test_helper.h"
#include <json/json.h>

struct TestResult {
    std::string opcode;
    int totalTests;
    int passedTests;
    int failedTests;
    bool allPassed;
};

// Get list of all opcode JSON files
std::vector<std::string> getOpcodeFiles(const std::string& directory) {
    std::vector<std::string> opcodes;
    
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".json") {
            opcodes.push_back(entry.path().stem().string());
        }
    }
    
    // Sort for consistent ordering
    std::sort(opcodes.begin(), opcodes.end());
    return opcodes;
}

// Test a single opcode with all its test cases
TestResult testOpcode(const std::string& opcodeHex, const std::string& jsonPath) {
    TestResult result;
    result.opcode = opcodeHex;
    result.totalTests = 0;
    result.passedTests = 0;
    result.failedTests = 0;
    
    // Parse JSON file
    Json::Value root;
    Json::Reader reader;
    std::ifstream file(jsonPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonPath << std::endl;
        result.allPassed = false;
        return result;
    }
    
    reader.parse(file, root);
    file.close();
    
    result.totalTests = root.size();
    
    // Run each test case
    for (int i = 0; i < root.size(); ++i) {
        // Create fresh emulator instance for each test
        Emu emu;
        emu.get_rom().create_blank_rom(0x8000);
        
        // Set initial state
        CpuTestHelper::setInitialState(emu.get_cpu(), emu.get_bus(), root[i]);
        
        // Execute instruction using actual execution path
        bool executed = emu.get_cpu().cpu_step();
        
        if (!executed) {
            std::cerr << "Opcode 0x" << opcodeHex << " test case " << i 
                      << ": Failed to execute" << std::endl;
            result.failedTests++;
            continue;
        }
        
        // Verify final state
        if (CpuTestHelper::verifyFinalState(emu.get_cpu(), emu.get_bus(), root[i])) {
            result.passedTests++;
        } else {
            std::cerr << "Opcode 0x" << opcodeHex << " test case " << i 
                      << " (" << root[i]["name"].asString() << "): FAILED" << std::endl;
            result.failedTests++;
        }
    }
    
    result.allPassed = (result.failedTests == 0);
    return result;
}

int main(int argc, char* argv[])
{
    std::string cwd = std::filesystem::current_path();
    std::string testDir = cwd + "/tests/SSTs/v1/";
    
    // Check if specific opcode was requested
    if (argc > 1) {
        std::string opcodeHex = argv[1];
        std::string jsonPath = testDir + opcodeHex + ".json";
        
        // Check if file exists
        if (!std::filesystem::exists(jsonPath)) {
            std::cerr << "Test file not found: " << jsonPath << std::endl;
            std::cerr << "Usage: " << argv[0] << " [opcode_hex]" << std::endl;
            std::cerr << "Example: " << argv[0] << " 01  (tests opcode 0x01)" << std::endl;
            return 1;
        }
        
        std::cout << "Testing opcode 0x" << opcodeHex << "...\n" << std::endl;
        TestResult result = testOpcode(opcodeHex, jsonPath);
        
        if (result.allPassed) {
            std::cout << "\n✓ All " << result.totalTests << " tests passed!" << std::endl;
            return 0;
        } else {
            std::cout << "\n✗ " << result.failedTests << "/" << result.totalTests 
                      << " tests failed" << std::endl;
            return 1;
        }
    }
    
    // Get all opcode files
    std::vector<std::string> opcodes = getOpcodeFiles(testDir);
    
    if (opcodes.empty()) {
        std::cerr << "No test files found in " << testDir << std::endl;
        return 1;
    }
    
    std::cout << "Found " << opcodes.size() << " opcode test files" << std::endl;
    std::cout << "Running tests...\n" << std::endl;
    
    // Run tests for all opcodes
    std::vector<TestResult> results;
    int totalOpcodesPassed = 0;
    int totalOpcodesFailed = 0;
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;
    
    for (const auto& opcodeHex : opcodes) {
        std::string jsonPath = testDir + opcodeHex + ".json";
        TestResult result = testOpcode(opcodeHex, jsonPath);
        results.push_back(result);
        
        if (result.allPassed) {
            totalOpcodesPassed++;
            std::cout << "✓ 0x" << std::setw(2) << std::setfill('0') << opcodeHex 
                      << ": " << result.passedTests << "/" << result.totalTests 
                      << " tests passed" << std::endl;
        } else {
            totalOpcodesFailed++;
            std::cout << "✗ 0x" << std::setw(2) << std::setfill('0') << opcodeHex 
                      << ": " << result.passedTests << "/" << result.totalTests 
                      << " tests passed (" << result.failedTests << " failed)" << std::endl;
        }
        
        totalTestsPassed += result.passedTests;
        totalTestsFailed += result.failedTests;
    }
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Opcodes tested: " << opcodes.size() << std::endl;
    std::cout << "Opcodes passed: " << totalOpcodesPassed << std::endl;
    std::cout << "Opcodes failed: " << totalOpcodesFailed << std::endl;
    std::cout << "Total test cases passed: " << totalTestsPassed << std::endl;
    std::cout << "Total test cases failed: " << totalTestsFailed << std::endl;
    
    // List failed opcodes
    if (totalOpcodesFailed > 0) {
        std::cout << "\nFailed opcodes:" << std::endl;
        for (const auto& result : results) {
            if (!result.allPassed) {
                std::cout << "  0x" << result.opcode << " (" << result.failedTests 
                          << "/" << result.totalTests << " failed)" << std::endl;
            }
        }
    }
    
    return (totalOpcodesFailed == 0) ? 0 : 1;
}