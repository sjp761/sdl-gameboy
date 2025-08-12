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

#ifdef OPCODETEST
void Cpu::set_opcode_test_data(Json::Value& root)
{
    Json::Value testCase = root[0];
    //std::cout << root[0].toStyledString() << std::endl;
    Json::Value initial = testCase["initial"];
    
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
}
#endif
