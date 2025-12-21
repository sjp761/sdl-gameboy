#pragma once
#include <cstdint>
#include "cpu_types.h"
#include <interrupts.h>
#include "cpu_tables.h"
class Ppu; 

struct Opcode
{
    uint8_t whole;
    uint8_t x;
    uint8_t y;
    uint8_t z;
    Opcode(uint8_t op) : whole(op), x((op >> 6) & 0x03), y((op >> 3) & 0x07), z(op & 0x07) {}
    Opcode() : whole(0), x(0), y(0), z(0) {}
};

struct cpu_registers
{
    uint8_t a;
    uint8_t f;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t pc;
    uint16_t sp;
};

class Bus; // Forward declaration
class Timer; // Forward declaration
class DMA; // Forward declaration

class Cpu
{
    private:
        Bus* bus;
        Timer* timer;
        DMA* dma;
        Ppu* ppu;
        // Helper methods for instruction tables
        uint8_t* get_r8_ptr(R8 reg);
        uint8_t read_r8(R8 reg);
        void write_r8(R8 reg, uint8_t value);
        
        uint16_t get_r16_group1(R16_Group1 reg);
        void set_r16_group1(R16_Group1 reg, uint16_t value);
        
        uint16_t get_r16_group2(R16_Group2 reg);
        void set_r16_group2(R16_Group2 reg, uint16_t value);
        
        uint16_t get_r16_group3(R16_Group3 reg);
        void set_r16_group3(R16_Group3 reg, uint16_t value);
        
        bool check_condition(ConditionCode cc);
        void execute_acc_flag_op(AccFlagOp op);
        void execute_alu_op(AluOp op, uint8_t operand);
        void execute_shift_rotate_op(ShiftRotateOp op, R8 reg);
        
        // Immediate data reading helpers
        uint8_t read_imm8();
        uint16_t read_imm16();
        int8_t read_imm_signed8();
        
        // Flag manipulation helpers
        void set_flag_z(bool value);
        void set_flag_n(bool value);
        void set_flag_h(bool value);
        void set_flag_c(bool value);
        bool get_flag_z() const;
        bool get_flag_n() const;
        bool get_flag_h() const;
        bool get_flag_c() const;
        void set_flags_dec(uint8_t result, uint8_t original);
        void set_flags_inc(uint8_t result, uint8_t original);
        
        // Stack operations
        void stack_push8(uint8_t value);
        void stack_push16(uint16_t value);
        uint8_t stack_pop8();
        uint16_t stack_pop16();
        
        // Instruction group handlers
        void execute_x0_instructions();
        void execute_x1_instructions();
        void execute_x2_instructions();
        void execute_x3_instructions();
        void execute_cb_instructions();
        
        // Specific instruction handlers
        void handle_ld_r16_imm16();
        void handle_ld_r8_r8();
        void handle_inc_r8();
        void handle_dec_r8();
        void handle_ld_r8_imm8();
        void handle_inc_r16();
        void handle_dec_r16();
        void handle_ld_indirect();
        void handle_alu_r8();
        void handle_alu_imm8();
        void handle_jp_and_interrupts();
        void handle_ret_conditional();
        void handle_pop_r16();
        void handle_push_r16();
        void handle_call_conditional();

        bool check_interrupt(Interrupts::InterruptMask it);
        void handle_interrupts();

        void interrupt_set_pc(uint16_t address);
        
    public:
        Cpu();
        // Set component pointers
        void set_cmp(Bus* bus_ptr, Timer* timer_ptr, DMA* dma_ptr, Ppu* ppu_ptr) { bus = bus_ptr; timer = timer_ptr; dma = dma_ptr; ppu = ppu_ptr; }
        void cpu_init();
        bool cpu_step();
        void fetch_data();
        void fetch_instruction();
        void execute_instruction();
        void read_serial_debug();
        void emu_cycles(int m_cycles);
        void request_interrupt(Interrupts::InterruptMask it);
        cpu_registers regs;
        uint16_t fetched_data;
        uint16_t mem_dest;
        Opcode opcode;
        bool halted;
        bool stepping;
        bool ime = 0; // Interrupt Master Enable flag
        bool ime_delay = 0;
};

