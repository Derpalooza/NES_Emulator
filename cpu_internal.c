#include "cpu_internal.h"

#include <stdio.h>

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

void cpu_print_instruction(uint16_t operand, char* instr_name, uint8_t addr_mode)
{
    char addressing_mode[8];
    switch (addr_mode)
    {
        case CPU_ADDR_MODE_INDIRECT_X:
        {
            sprintf(addressing_mode, "INX");
            break;
        }
        case CPU_ADDR_MODE_ZEROPAGE:
        {
            sprintf(addressing_mode, "ZPG");
            break;
        }
        case CPU_ADDR_MODE_IMMEDIATE:
        {
            sprintf(addressing_mode, "IMM");
            break;
        }
        case CPU_ADDR_MODE_ABSOLUTE:
        {
            sprintf(addressing_mode, "ABS");
            break;
        }
        case CPU_ADDR_MODE_INDIRECT_Y:
        {
            sprintf(addressing_mode, "INY");
            break;
        }
        case CPU_ADDR_MODE_ZERO_PAGE_IDX:
        {
            sprintf(addressing_mode, "ZPX");
            break;
        }
        case CPU_ADDR_MODE_ABSOLUTE_Y:
        {
            sprintf(addressing_mode, "ABY");
            break;
        }
        case CPU_ADDR_MODE_ABSOLUTE_X:
        {
            sprintf(addressing_mode, "ABX");
            break;
        }
        case CPU_ADDR_MODE_IMPLIED:
        {
            sprintf(addressing_mode, "IMP");
            break;
        }
        case CPU_ADDR_MODE_RELATIVE:
        {
            sprintf(addressing_mode, "REL");
            break;
        }
        default:
            printf("Invalid addressing mode %d\n", addr_mode);
            break;
    }

    if (addr_mode == CPU_ADDR_MODE_IMPLIED)
    {
        printf("%s: %s\n\n", addressing_mode, instr_name);
    }
    else
    {
        printf("%s: %s 0x%x\n\n", addressing_mode, instr_name, operand);
    }
}