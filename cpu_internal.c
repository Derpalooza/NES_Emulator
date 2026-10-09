#include "cpu_internal.h"

// Set zero flag if data is zero
static void check_status_zero_flag(cpu_t* cpu, uint8_t data)
{
    if (data == 0)
    {
        cpu->P |= STATUS_ZERO_FLAG_MASK;
    }
    else
    {
        cpu->P &= ~STATUS_ZERO_FLAG_MASK;
    }
}

// Set negative flag if highest bit is 1
static void check_status_negative_flag(cpu_t* cpu, uint8_t data)
{
    if ((data & 0x80) != 0)
    {
        cpu->P |= STATUS_NEGATIVE_MASK;
    }
    else
    {
        cpu->P &= ~STATUS_NEGATIVE_MASK;
    }
}

void cpu_set_register(cpu_t* cpu, uint8_t* dst, uint8_t src)
{
    check_status_zero_flag(cpu, src);
    check_status_negative_flag(cpu, src);

    *dst = src;
}

uint8_t cpu_get_opcode_addressing_mode(uint8_t opcode)
{
    // Obtain the addressing mode from bits 2-4 in the opcode
	uint8_t addr_mode = (opcode >> 2) & 0x7;
    return addr_mode;
}

uint8_t cpu_get_target_register(uint8_t opcode)
{
    // Obtain the target register from bits 0-1 in the opcode
	uint8_t target_reg = opcode & 0x3;
    return target_reg;
}
