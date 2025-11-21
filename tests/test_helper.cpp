#include "test_helper.h"
#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <iomanip>

void CpuTestHelper::setInitialState(Cpu& cpu, Bus& bus, const Json::Value& testCase) {
    const Json::Value& initial = testCase["initial"];
    
    // Set CPU registers
    cpu.regs.pc = initial["pc"].asUInt();
    cpu.regs.sp = initial["sp"].asUInt();
    cpu.regs.a = initial["a"].asUInt();
    cpu.regs.b = initial["b"].asUInt();
    cpu.regs.c = initial["c"].asUInt();
    cpu.regs.d = initial["d"].asUInt();
    cpu.regs.e = initial["e"].asUInt();
    cpu.regs.f = initial["f"].asUInt();
    cpu.regs.h = initial["h"].asUInt();
    cpu.regs.l = initial["l"].asUInt();
    
    // Set IME flag if present
    if (initial.isMember("ime")) {
        cpu.ime = initial["ime"].asUInt() != 0;
    }
    
    // Set IE register (0xFFFF) if present
    if (initial.isMember("ie")) {
        bus.bus_write(0xFFFF, initial["ie"].asUInt());
    }
    
    // Write initial RAM values to memory
    const Json::Value& ramArray = initial["ram"];
    for (const auto& ramEntry : ramArray) {
        uint16_t addr = ramEntry[0].asUInt();
        uint8_t value = ramEntry[1].asUInt();
        bus.bus_write(addr, value);
    }
}

bool CpuTestHelper::verifyFinalState(Cpu& cpu, Bus& bus, const Json::Value& testCase) {
    const Json::Value& expected = testCase["final"];
    bool success = true;
    
    // Check PC
    if (cpu.regs.pc != expected["pc"].asUInt()) {
        std::cerr << "  PC mismatch: expected 0x" << std::hex << std::setw(4) << std::setfill('0')
                  << expected["pc"].asUInt() << ", got 0x" << cpu.regs.pc << std::dec << std::endl;
        success = false;
    }
    
    // Check SP
    if (cpu.regs.sp != expected["sp"].asUInt()) {
        std::cerr << "  SP mismatch: expected 0x" << std::hex << std::setw(4) << std::setfill('0')
                  << expected["sp"].asUInt() << ", got 0x" << cpu.regs.sp << std::dec << std::endl;
        success = false;
    }
    
    // Check A
    if (cpu.regs.a != expected["a"].asUInt()) {
        std::cerr << "  A mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["a"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.a) << std::dec << std::endl;
        success = false;
    }
    
    // Check B
    if (cpu.regs.b != expected["b"].asUInt()) {
        std::cerr << "  B mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["b"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.b) << std::dec << std::endl;
        success = false;
    }
    
    // Check C
    if (cpu.regs.c != expected["c"].asUInt()) {
        std::cerr << "  C mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["c"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.c) << std::dec << std::endl;
        success = false;
    }
    
    // Check D
    if (cpu.regs.d != expected["d"].asUInt()) {
        std::cerr << "  D mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["d"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.d) << std::dec << std::endl;
        success = false;
    }
    
    // Check E
    if (cpu.regs.e != expected["e"].asUInt()) {
        std::cerr << "  E mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["e"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.e) << std::dec << std::endl;
        success = false;
    }
    
    // Check F
    if (cpu.regs.f != expected["f"].asUInt()) {
        std::cerr << "  F mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["f"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.f) << std::dec << std::endl;
        success = false;
    }
    
    // Check H
    if (cpu.regs.h != expected["h"].asUInt()) {
        std::cerr << "  H mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << expected["h"].asUInt() << ", got 0x" << static_cast<int>(cpu.regs.h) << std::dec << std::endl;
        success = false;
    }
    
    // Check L
    if (cpu.regs.l != expected["l"].asUInt()) {
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
        uint8_t expected_ie = expected["ie"].asUInt();
        uint8_t actual_ie = bus.bus_read(0xFFFF);
        if (actual_ie != expected_ie) {
            std::cerr << "  IE mismatch: expected 0x" << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(expected_ie) << ", got 0x" << static_cast<int>(actual_ie) << std::dec << std::endl;
            success = false;
        }
    }
    
    // TODO: Check RAM state if needed
    
    return success;
}
