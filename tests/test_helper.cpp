#include "test_helper.h"
#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <iomanip>

void CpuTestHelper::setInitialState(Cpu& cpu, Bus& bus, const Json::Value& testCase) {
    const Json::Value& initial = testCase["initial"];
    
    // Set CPU registers
    cpu.regs.pc = static_cast<uint16_t>(initial["pc"].asUInt());
    cpu.regs.sp = static_cast<uint16_t>(initial["sp"].asUInt());
    cpu.regs.a = static_cast<uint8_t>(initial["a"].asUInt());
    cpu.regs.b = static_cast<uint8_t>(initial["b"].asUInt());
    cpu.regs.c = static_cast<uint8_t>(initial["c"].asUInt());
    cpu.regs.d = static_cast<uint8_t>(initial["d"].asUInt());
    cpu.regs.e = static_cast<uint8_t>(initial["e"].asUInt());
    cpu.regs.f = static_cast<uint8_t>(initial["f"].asUInt());
    cpu.regs.h = static_cast<uint8_t>(initial["h"].asUInt());
    cpu.regs.l = static_cast<uint8_t>(initial["l"].asUInt());
    
    // Set IME flag if present
    if (initial.isMember("ime")) {
        cpu.ime = initial["ime"].asUInt() != 0;
    }
    
    // Set IE register (0xFFFF) if present
    if (initial.isMember("ie")) {
        bus.bus_write(0xFFFF, static_cast<uint8_t>(initial["ie"].asUInt()));
    }
    
    // Write initial RAM values to memory
    const Json::Value& ramArray = initial["ram"];
    for (const auto& ramEntry : ramArray) {
        uint16_t addr = static_cast<uint16_t>(ramEntry[0].asUInt());
        uint8_t value = static_cast<uint8_t>(ramEntry[1].asUInt());
        bus.bus_write(addr, value);
    }
}

bool CpuTestHelper::verifyFinalState(Cpu& cpu, Bus& bus, const Json::Value& testCase) {
    const Json::Value& expected = testCase["final"];
    bool success = true;
    
    // Check PC
    if (cpu.regs.pc != static_cast<uint16_t>(expected["pc"].asUInt())) {
        std::cerr << "  PC mismatch: expected 0x" << std::hex << std::setw(4) << std::setfill('0')
                  << expected["pc"].asUInt() << ", got 0x" << cpu.regs.pc << std::dec << std::endl;
        success = false;
    }
    
    // Check SP
    if (cpu.regs.sp != static_cast<uint16_t>(expected["sp"].asUInt())) {
        std::cerr << "  SP mismatch: expected 0x" << std::hex << std::setw(4) << std::setfill('0')
                  << expected["sp"].asUInt() << ", got 0x" << cpu.regs.sp << std::dec << std::endl;
        success = false;
    }
    
    // Check A
    if (cpu.regs.a != static_cast<uint8_t>(expected["a"].asUInt())) {
        std::cerr << "  A mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["a"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.a) << std::dec << std::endl;
        success = false;
    }
    
    // Check B
    if (cpu.regs.b != static_cast<uint8_t>(expected["b"].asUInt())) {
        std::cerr << "  B mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["b"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.b) << std::dec << std::endl;
        success = false;
    }
    
    // Check C
    if (cpu.regs.c != static_cast<uint8_t>(expected["c"].asUInt())) {
        std::cerr << "  C mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["c"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.c) << std::dec << std::endl;
        success = false;
    }
    
    // Check D
    if (cpu.regs.d != static_cast<uint8_t>(expected["d"].asUInt())) {
        std::cerr << "  D mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["d"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.d) << std::dec << std::endl;
        success = false;
    }
    
    // Check E
    if (cpu.regs.e != static_cast<uint8_t>(expected["e"].asUInt())) {
        std::cerr << "  E mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["e"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.e) << std::dec << std::endl;
        success = false;
    }
    
    // Check F
    if (cpu.regs.f != static_cast<uint8_t>(expected["f"].asUInt())) {
        std::cerr << "  F mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["f"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.f) << std::dec << std::endl;
        success = false;
    }
    
    // Check H
    if (cpu.regs.h != static_cast<uint8_t>(expected["h"].asUInt())) {
        std::cerr << "  H mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["h"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.h) << std::dec << std::endl;
        success = false;
    }
    
    // Check L
    if (cpu.regs.l != static_cast<uint8_t>(expected["l"].asUInt())) {
        std::cerr << "  L mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["l"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.l) << std::dec << std::endl;
        success = false;
    }
    
    // Check IME flag if present
    if (expected.isMember("ime")) {
        bool expected_ime = expected["ime"].asUInt() != 0;
        if (cpu.ime != expected_ime) {
            std::cerr << "  IME mismatch: expected " << expected_ime 
                      << ", got " << cpu.ime << std::endl;
            success = false;
        }
    }
    
    // Check IE register (0xFFFF) if present
    if (expected.isMember("ie")) {
        uint8_t expected_ie = static_cast<uint8_t>(expected["ie"].asUInt());
        uint8_t actual_ie = bus.bus_read(0xFFFF);
        if (actual_ie != expected_ie) {
            std::cerr << "  IE mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(expected_ie) << ", got 0x" << static_cast<int>(actual_ie) << std::dec << std::endl;
            success = false;
        }
    }
    
    // Check RAM state if present: expected["ram"] is an array of [addr, value]
    if (expected.isMember("ram")) {
        const Json::Value& ramArray = expected["ram"];
        for (const auto& ramEntry : ramArray) {
            if (ramEntry.size() < 2) continue;
            uint16_t addr = static_cast<uint16_t>(ramEntry[0].asUInt());
            uint8_t expected_val = static_cast<uint8_t>(ramEntry[1].asUInt());
            uint8_t actual_val = bus.bus_read(addr);
            if (actual_val != expected_val) {
                std::cerr << "  RAM mismatch at 0x" << std::hex << std::setw(4) << std::setfill('0')
                          << addr << ": expected 0x" << std::setw(2) << static_cast<int>(expected_val)
                          << ", got 0x" << std::setw(2) << static_cast<int>(actual_val) << std::dec << std::endl;
                success = false;
            }
        }
    }
    return success;
}
