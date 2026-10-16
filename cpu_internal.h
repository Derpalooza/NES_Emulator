#ifndef _CPU_INTERNAL_H_
#define _CPU_INTERNAL_H_

// Internal CPU helper functions

#include "cpu.h"

typedef enum
{
    CPU_ADDR_MODE_INDIRECT_X = 0,
    CPU_ADDR_MODE_ZEROPAGE,
    CPU_ADDR_MODE_IMMEDIATE,
    CPU_ADDR_MODE_ABSOLUTE,
    CPU_ADDR_MODE_INDIRECT_Y,
    CPU_ADDR_MODE_ZERO_PAGE_IDX,
    CPU_ADDR_MODE_ABSOLUTE_Y,
    CPU_ADDR_MODE_ABSOLUTE_X,
    CPU_ADDR_MODE_IMPLIED,
    CPU_ADDR_MODE_RELATIVE,
    NUM_CPU_ADDR_MODES
} cpu_addr_mode_t;

typedef enum
{
    CPU_REGISTER_A = 0,
    CPU_REGISTER_X,
    CPU_REGISTER_Y,
    CPU_REGISTER_SP,
    NUM_CPU_REGISTERS
} cpu_register_t;

// Populates source register with contents of destination register, and sets status bits
// Registers MUST be set through this function. They must not be modified directly
status_t cpu_set_register(cpu_t* cpu, cpu_register_t reg, uint8_t src);


// Pure helper functions

// Obtain the addressing mode of the given opcode and returns a CPU_ADDR_MODE_* macro
cpu_addr_mode_t cpu_get_opcode_addressing_mode(uint8_t opcode);

status_t cpu_get_opcode_register(uint8_t opcode, cpu_register_t* reg);

// Obtain the target register of the given opcode and returns a CPU_REGISTER_* macro
uint8_t cpu_get_target_register(uint8_t opcode);

// Print the instruction mnemonic for debugging purposes
void cpu_print_instruction(uint16_t operand, char* instr_name, uint8_t addr_mode);

#endif