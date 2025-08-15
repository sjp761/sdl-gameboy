#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include "instruction.h"
#include <iostream>

#ifdef OPCODETEST
#include <json/json.h>
#endif

void Cpu::cpu_init()
{
    regs.pc = 0x100;
}

bool Cpu::cpu_step()
{
    if (!halted)
    {
        fetch_instruction();
        instruction = Instruction(opcode);
        if (instruction.type == IN_NONE)
        {
            std::cerr << "Unknown opcode: " << std::hex << static_cast<int>(opcode.whole) << std::dec << std::endl;
            return false; // Invalid instruction, do not proceed
        }
        fetch_data();
        return true; // Indicate that a step was executed
    }
    return false; // Indicate that no step was executed
}

void Cpu::fetch_data()
{
    #ifdef OPCODETEST
        regs.pc++; //We dont call fetch instruction so increment PC here
    #endif
   if (instruction.oprnd == AM_R_IMM16)
   {
    fetched_data = (Emu::cmp.bus.bus_read(regs.pc) << 8) | (Emu::cmp.bus.bus_read(regs.pc + 1));
    regs.pc += 2;
   }
}

void Cpu::emu_cycles()
{
}

void Cpu::fetch_instruction()
{

    opcode = Opcode(Emu::cmp.bus.bus_read(regs.pc++));
    std::cout << "Fetched instruction: " << static_cast<int>(opcode.whole)
              << " at PC: " << std::hex << regs.pc - 1 << std::dec << std::endl;
}

void Cpu::execute_instruction()
{
    if (opcode.whole == 0x01) // LD BC, d16
    {
        std::cout << "Executing LD BC, d16 with fetched_data: " << std::hex << fetched_data << std::dec << std::endl;
        regs.c = (fetched_data >> 8) & 0xFF;
        regs.b = fetched_data & 0xFF;
        std::cout << "Set B=" << static_cast<int>(regs.b) << ", C=" << static_cast<int>(regs.c) << std::endl;
    }
}

#ifdef OPCODETEST
void Cpu::set_opcode_test_data(Json::Value& root, int index)
{
    Json::Value testCase = root[index];
    
    // CPU registers
    regs.pc = testCase["initial"]["pc"].asUInt();
    regs.sp = testCase["initial"]["sp"].asUInt();
    regs.a = testCase["initial"]["a"].asUInt();
    regs.b = testCase["initial"]["b"].asUInt();
    regs.c = testCase["initial"]["c"].asUInt();
    regs.d = testCase["initial"]["d"].asUInt();
    regs.e = testCase["initial"]["e"].asUInt();
    regs.f = testCase["initial"]["f"].asUInt();
    regs.h = testCase["initial"]["h"].asUInt();
    regs.l = testCase["initial"]["l"].asUInt();
    
    std::cout << "Initial CPU state: "
              << "PC: " << std::hex << regs.pc
              << ", SP: " << std::dec << regs.sp
              << ", A: " << static_cast<int>(regs.a)
              << ", B: " << static_cast<int>(regs.b)
              << ", C: " << static_cast<int>(regs.c)
              << ", D: " << static_cast<int>(regs.d)
              << ", E: " << static_cast<int>(regs.e)
              << ", F: " << static_cast<int>(regs.f)
              << ", H: " << static_cast<int>(regs.h)
              << ", L: " << static_cast<int>(regs.l) << std::endl;
    // Write the initial RAM value to memory using bus_write
    const Json::Value& ramArray = testCase["initial"]["ram"];
    for (const auto& ramEntry : ramArray)
    {
        uint16_t ram_addr = ramEntry[0].asUInt();
        uint8_t ram_value = ramEntry[1].asUInt();
        Emu::cmp.bus.bus_write(ram_addr, ram_value);
        std::cout << "Initial ram state: addr=" << std::hex << ram_addr << ", value=" << static_cast<int>(ram_value) << std::dec << std::endl;
    }
}

bool Cpu::check_opcode_data(Json::Value& root, int index)
{
    Json::Value testCase = root[index];
    Json::Value expected = testCase["final"];
    

        if (regs.pc != expected["pc"].asUInt())
        {
            std::cerr << "PC mismatch: expected " << std::hex << expected["pc"].asUInt() 
                    << ", got " << regs.pc << std::dec << std::endl;
            return false;
        }
        if (regs.sp != expected["sp"].asUInt())
        {
            std::cerr << "SP mismatch: expected " << std::hex << expected["sp"].asUInt()
                      << ", got " << regs.sp << std::dec << std::endl;
            return false;
        }
        if (regs.a != expected["a"].asUInt())
        {
            std::cerr << "A mismatch: expected " << std::hex << expected["a"].asUInt()
                      << ", got " << static_cast<int>(regs.a) << std::dec << std::endl;
            return false;
        }
        if (regs.b != expected["b"].asUInt())
        {
            std::cerr << "B mismatch: expected " << std::hex << expected["b"].asUInt()
                      << ", got " << static_cast<int>(regs.b) << std::dec << std::endl;
            return false;
        }
        if (regs.c != expected["c"].asUInt())
        {
            std::cerr << "C mismatch: expected " << std::hex << expected["c"].asUInt()
                      << ", got " << static_cast<int>(regs.c) << std::dec << std::endl;
            return false;
        }
        if (regs.d != expected["d"].asUInt())
        {
            std::cerr << "D mismatch: expected " << std::hex << expected["d"].asUInt()
                      << ", got " << static_cast<int>(regs.d) << std::dec << std::endl;
            return false;
        }
        if (regs.e != expected["e"].asUInt())
        {
            std::cerr << "E mismatch: expected " << std::hex << expected["e"].asUInt()
                      << ", got " << static_cast<int>(regs.e) << std::dec << std::endl;
            return false;
        }
        if (regs.f != expected["f"].asUInt())
        {
            std::cerr << "F mismatch: expected " << std::hex << expected["f"].asUInt()
                      << ", got " << static_cast<int>(regs.f) << std::dec << std::endl;
            return false;
        }
        if (regs.h != expected["h"].asUInt())
        {
            std::cerr << "H mismatch: expected " << std::hex << expected["h"].asUInt()
                      << ", got " << static_cast<int>(regs.h) << std::dec << std::endl;
            return false;
        }
        if (regs.l != expected["l"].asUInt())
        {
            std::cerr << "L mismatch: expected " << std::hex << expected["l"].asUInt()
                      << ", got " << static_cast<int>(regs.l) << std::dec << std::endl;
            return false;
        }

    return true;
}
#endif
