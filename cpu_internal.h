#ifndef _CPU_INTERNAL_H_
#define _CPU_INTERNAL_H_

// Internal CPU helper functions

#include "cpu.h"

// Register identifiers (Obtained from opcode bits 0-1)
#define CPU_REGISTER_Y		0x0
#define CPU_REGISTER_A		0x1
#define CPU_REGISTER_X		0x2

// Addressing mode identifiers (Obtained from opcode bits 2-4)
#define CPU_ADDR_MODE_INDIRECT_X		0x0
#define CPU_ADDR_MODE_ZEROPAGE			0x1
#define CPU_ADDR_MODE_IMMEDIATE			0x2
#define CPU_ADDR_MODE_ABSOLUTE			0x3
#define CPU_ADDR_MODE_INDIRECT_Y		0x4
#define CPU_ADDR_MODE_ZERO_PAGE_IDX		0x5
#define CPU_ADDR_MODE_ABSOLUTE_Y		0x6
#define CPU_ADDR_MODE_ABSOLUTE_X		0x7
#define CPU_ADDR_MODE_IMPLIED           0x8
#define CPU_ADDR_MODE_RELATIVE          0x9

// Populates source register with contents of destination register, and sets status bits
// Registers MUST be set through this function. They must not be modified directly
void cpu_set_register(cpu_t* cpu, uint8_t* dst, uint8_t src);


// Pure helper functions

// Obtain the addressing mode of the given opcode and returns a CPU_ADDR_MODE_* macro
uint8_t cpu_get_opcode_addressing_mode(uint8_t opcode);

// Obtain the target register of the given opcode and returns a CPU_REGISTER_* macro
uint8_t cpu_get_target_register(uint8_t opcode);

// Print the instruction mnemonic for debugging purposes
void cpu_print_instruction(uint16_t operand, char* instr_name, uint8_t addr_mode);

#endif