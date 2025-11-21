#pragma once
#include <json/json.h>
#include <cstdint>

// Forward declarations
class Cpu;
class Bus;

class CpuTestHelper {
public:
    // Set CPU and memory state from JSON test data
    static void setInitialState(Cpu& cpu, Bus& bus, const Json::Value& testCase);
    
    // Verify final CPU and memory state against expected JSON data
    static bool verifyFinalState(Cpu& cpu, Bus& bus, const Json::Value& testCase);
};
