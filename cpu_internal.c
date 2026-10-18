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

status_t cpu_set_register(cpu_t* cpu, cpu_register_t reg, uint8_t src)
{
    if (reg >= NUM_CPU_REGISTERS)
    {
        printf("Invalid register %d\n", reg);
        return STATUS_EINVALIDPARAMS;
    }

    if (reg != CPU_REGISTER_SP)
    {
        // The status bits are not sensitive to writes to the stack pointer
        check_status_zero_flag(cpu, src);
        check_status_negative_flag(cpu, src);
    }

	switch (reg)
	{
		case CPU_REGISTER_Y:
			cpu->Y = src;
			break;
		case CPU_REGISTER_A:
			cpu->A = src;
			break;
		case CPU_REGISTER_X:
			cpu->X = src;
			break;
        case CPU_REGISTER_SP:
            cpu->SP = src;
            break;
	}

    return STATUS_SUCCESS;
}

uint8_t cpu_add_data(cpu_t* cpu, uint8_t A, uint8_t B)
{
    uint8_t result = (uint8_t)A + (uint8_t)B;

    check_status_zero_flag(cpu, result);
    check_status_negative_flag(cpu, result);

    return result;
}

cpu_addr_mode_t cpu_get_opcode_addressing_mode(uint8_t opcode)
{
    // Obtain the addressing mode from bits 2-4 in the opcode
	uint8_t addr_mode = (opcode >> 2) & 0x7;

    switch (addr_mode)
    {
        case 0x0: return CPU_ADDR_MODE_INDIRECT_X;
        case 0x1: return CPU_ADDR_MODE_ZEROPAGE;
        case 0x2: return CPU_ADDR_MODE_IMMEDIATE;
        case 0x3: return CPU_ADDR_MODE_ABSOLUTE;
        case 0x4: return CPU_ADDR_MODE_INDIRECT_Y;
        case 0x5: return CPU_ADDR_MODE_ZERO_PAGE_IDX;
        case 0x6: return CPU_ADDR_MODE_ABSOLUTE_Y;
        case 0x7: return CPU_ADDR_MODE_ABSOLUTE_X;
    }
}

status_t cpu_get_opcode_register(uint8_t opcode, cpu_register_t* reg)
{
    // Obtain the register from bits 0-1 in the opcode
	uint8_t target_register = cpu_get_target_register(opcode);

    switch (target_register)
    {
        case 0x0:
            *reg = CPU_REGISTER_Y;
            break;
        case 0x1:
            *reg = CPU_REGISTER_A;
            break;
        case 0x2:
            *reg = CPU_REGISTER_X;
            break;
        default:
            printf("Invalid register %d for opcode 0x%x", target_register);
            return STATUS_EINVALIDPARAMS;
    }

    return STATUS_SUCCESS;
}

uint8_t cpu_get_target_register(uint8_t opcode)
{
    // Obtain the target register from bits 0-1 in the opcode
	uint8_t target_reg = opcode & 0x3;
    return target_reg;
}

void cpu_print_instruction(uint16_t effective_addr, uint8_t operand, char* instr_name, uint8_t addr_mode)
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
        printf("%s: %s\n", addressing_mode, instr_name);
    }
    else
    {
        printf("%s: %s 0x%x 0x%x\n", addressing_mode, instr_name, effective_addr, operand);
    }
}