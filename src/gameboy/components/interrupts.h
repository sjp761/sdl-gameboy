#pragma once
#include <cstdint>
#include <unordered_map>

namespace Interrupts
{
enum class InterruptMask : uint8_t 
{
    IT_VBlank = 0x01,    // Vertical Blank Interrupt
    IT_LCDStat = 0x02,   // LCD STAT Interrupt
    IT_Timer = 0x04,     // Timer Overflow Interrupt
    IT_Serial = 0x08,    // Serial Transfer Completion Interrupt
    IT_Joypad = 0x10     // Joypad Press Interrupt
};


inline std::unordered_map<InterruptMask, uint16_t> InterruptPC = //
{
    { InterruptMask::IT_VBlank, 0x40 },
    { InterruptMask::IT_LCDStat, 0x48 },
    { InterruptMask::IT_Timer, 0x50 },
    { InterruptMask::IT_Serial, 0x58 },
    { InterruptMask::IT_Joypad, 0x60 }
};

} // namespace interrupts

