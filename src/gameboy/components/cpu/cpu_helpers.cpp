#include "cpu.h"
#include "bus.h"

// ===== R8 Helper Methods =====
uint8_t* Cpu::get_r8_ptr(R8 reg) {
    uint8_t* table[8] = {
        &regs.b, &regs.c, &regs.d, &regs.e,
        &regs.h, &regs.l, nullptr, &regs.a
    };
    return table[static_cast<uint8_t>(reg)];
}

uint8_t Cpu::read_r8(R8 reg) {
    if (reg == R8::HL_IND) {
        uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
        return bus.bus_read(hl);
    }
    return *get_r8_ptr(reg);
}

void Cpu::write_r8(R8 reg, uint8_t value) {
    if (reg == R8::HL_IND) {
        uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
        bus.bus_write(hl, value);
    } else {
        *get_r8_ptr(reg) = value;
    }
}

// ===== R16 Group 1 Helper Methods (BC, DE, HL, SP) =====
uint16_t Cpu::get_r16_group1(R16_Group1 reg) {
    switch (reg) {
        case R16_Group1::BC: return (static_cast<uint16_t>(regs.b) << 8) | regs.c;
        case R16_Group1::DE: return (static_cast<uint16_t>(regs.d) << 8) | regs.e;
        case R16_Group1::HL: return (static_cast<uint16_t>(regs.h) << 8) | regs.l;
        case R16_Group1::SP: return regs.sp;
    }
    return 0;
}

void Cpu::set_r16_group1(R16_Group1 reg, uint16_t value) {
    uint8_t low = static_cast<uint8_t>(value);
    uint8_t high = static_cast<uint8_t>(value >> 8);
    
    switch (reg) {
        case R16_Group1::BC: regs.c = low; regs.b = high; break;
        case R16_Group1::DE: regs.e = low; regs.d = high; break;
        case R16_Group1::HL: regs.l = low; regs.h = high; break;
        case R16_Group1::SP: regs.sp = value; break;
    }
}

// ===== R16 Group 2 Helper Methods (BC, DE, HL+, HL-) =====
uint16_t Cpu::get_r16_group2(R16_Group2 reg) {
    uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
    
    switch (reg) {
        case R16_Group2::BC: return (static_cast<uint16_t>(regs.b) << 8) | regs.c;
        case R16_Group2::DE: return (static_cast<uint16_t>(regs.d) << 8) | regs.e;
        case R16_Group2::HL_INC: {
            uint16_t result = hl;
            hl++;
            regs.h = hl >> 8;
            regs.l = hl & 0xFF;
            return result;
        }
        case R16_Group2::HL_DEC: {
            uint16_t result = hl;
            hl--;
            regs.h = hl >> 8;
            regs.l = hl & 0xFF;
            return result;
        }
    }
    return 0;
}

void Cpu::set_r16_group2(R16_Group2 reg, uint16_t value) {
    // Similar to get but reversed for stores
    uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
    
    switch (reg) {
        case R16_Group2::BC: regs.c = value & 0xFF; regs.b = value >> 8; break;
        case R16_Group2::DE: regs.e = value & 0xFF; regs.d = value >> 8; break;
        case R16_Group2::HL_INC:
            hl++;
            regs.h = hl >> 8;
            regs.l = hl & 0xFF;
            break;
        case R16_Group2::HL_DEC:
            hl--;
            regs.h = hl >> 8;
            regs.l = hl & 0xFF;
            break;
    }
}

// ===== R16 Group 3 Helper Methods (BC, DE, HL, AF) =====
uint16_t Cpu::get_r16_group3(R16_Group3 reg) {
    switch (reg) {
        case R16_Group3::BC: return (static_cast<uint16_t>(regs.b) << 8) | regs.c;
        case R16_Group3::DE: return (static_cast<uint16_t>(regs.d) << 8) | regs.e;
        case R16_Group3::HL: return (static_cast<uint16_t>(regs.h) << 8) | regs.l;
        case R16_Group3::AF: return (static_cast<uint16_t>(regs.a) << 8) | regs.f;
    }
    return 0;
}

void Cpu::set_r16_group3(R16_Group3 reg, uint16_t value) {
    uint8_t low = static_cast<uint8_t>(value);
    uint8_t high = static_cast<uint8_t>(value >> 8);
    
    switch (reg) {
        case R16_Group3::BC: regs.c = low; regs.b = high; break;
        case R16_Group3::DE: regs.e = low; regs.d = high; break;
        case R16_Group3::HL: regs.l = low; regs.h = high; break;
        case R16_Group3::AF: regs.f = low & 0xF0; regs.a = high; break; // Mask lower 4 bits of F
    }
}

// ===== Condition Code Helper =====
bool Cpu::check_condition(ConditionCode cc) {
    switch (cc) {
        case ConditionCode::NZ: return !(regs.f & 0x80); // Zero flag not set
        case ConditionCode::Z:  return (regs.f & 0x80);  // Zero flag set
        case ConditionCode::NC: return !(regs.f & 0x10); // Carry flag not set
        case ConditionCode::C:  return (regs.f & 0x10);  // Carry flag set
    }
    return false;
}

// ===== Immediate Data Reading Helpers =====
uint8_t Cpu::read_imm8() {
    return bus.bus_read(regs.pc++);
}

uint16_t Cpu::read_imm16() {
    uint8_t low = bus.bus_read(regs.pc++);
    uint8_t high = bus.bus_read(regs.pc++);
    return (static_cast<uint16_t>(high) << 8) | low;
}

int8_t Cpu::read_imm_signed8() {
    return static_cast<int8_t>(bus.bus_read(regs.pc++));
}

// ===== Flag Manipulation Helpers =====
void Cpu::set_flag_z(bool value) {
    regs.f = value ? (regs.f | 0x80) : (regs.f & ~0x80);
}

void Cpu::set_flag_n(bool value) {
    regs.f = value ? (regs.f | 0x40) : (regs.f & ~0x40);
}

void Cpu::set_flag_h(bool value) {
    regs.f = value ? (regs.f | 0x20) : (regs.f & ~0x20);
}

void Cpu::set_flag_c(bool value) {
    regs.f = value ? (regs.f | 0x10) : (regs.f & ~0x10);
}

bool Cpu::get_flag_z() const {
    return regs.f & 0x80;
}

bool Cpu::get_flag_n() const {
    return regs.f & 0x40;
}

bool Cpu::get_flag_h() const {
    return regs.f & 0x20;
}

bool Cpu::get_flag_c() const {
    return regs.f & 0x10;
}

void Cpu::set_flags_dec(uint8_t result, uint8_t original) {
    set_flag_z(result == 0);
    set_flag_n(true);
    set_flag_h((result & 0x0F) == 0x0F); // Half-carry on borrow from bit 4
    // Carry flag unchanged
}

void Cpu::set_flags_inc(uint8_t result, uint8_t original) {
    set_flag_z(result == 0);
    set_flag_n(false);
    set_flag_h((original & 0x0F) == 0x0F); // Half-carry on carry from bit 3
    // Carry flag unchanged
}
