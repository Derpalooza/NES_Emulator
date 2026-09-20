#include "opcode.h"

#include <stdio.h>

#include "cpu.h"
#include "memory.h"

status_t get_operand_absolute(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;

    if (cpu->state == CPU_STATE_FETCH)
    {
        cpu->remaining_cycles = 2;
        cpu->PC++;
        status = STATUS_SUCCESS;
    }
    else
    {
        // Read operand byte
        uint8_t operand_byte = 0;
        status = memory_read_byte(memory, cpu->PC, &operand_byte);
        if (status != STATUS_SUCCESS)
        {
            printf("memory_read_byte failed with error %d\n", status);
        }

        switch (cpu->remaining_cycles)
        {
            case 2:
                // Operand represents the lower byte of the address
                cpu->effective_address = operand_byte;
                cpu->PC++;
                status = STATUS_INPROGRESS;
                break;
            case 1:
                // Operand represents the higher byte of the address
                cpu->effective_address |= (operand_byte << 8);
                cpu->PC++;
                status = STATUS_SUCCESS;
                break;
            default:
                printf("Invalid number of remaining cycles\n");
                status = STATUS_EINVALIDPARAMS;
                break;
        }

        cpu->remaining_cycles--;
    }

    return status;
}

// Flag (Processor Status) Instructions
// TODO: See if these can be joined into one function

status_t proc_flag_set_clear(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            cpu->state = CPU_STATE_EXECUTE;
            cpu->PC++;
            break;
        case CPU_STATE_EXECUTE:
            switch (cpu->current_opcode)
            {
                case 0x18:
                    cpu->P &= ~STATUS_CARRY_FLAG_MASK;
                    printf("CLC\n");
                    break;
                case 0x38:
                    cpu->P |= STATUS_CARRY_FLAG_MASK;
                    printf("SEC\n");
                    break;
                case 0x58:
                    cpu->P &= ~STATUS_INTERRUPT_DISABLE_MASK;
                    printf("CLI\n");
                    break;
                case 0x78:
                    cpu->P |= STATUS_INTERRUPT_DISABLE_MASK;
                    printf("SEI\n");
                    break;
                case 0xB8:
                    cpu->P &= ~STATUS_OVERFLOW_MASK;
                    printf("CLV\n");
                    break;
                case 0xD8:
                    cpu->P &= ~STATUS_DECIMAL_MODE_MASK;
                    printf("CLD\n");
                    break;
                case 0xF8:
                    cpu->P |= STATUS_DECIMAL_MODE_MASK;
                    printf("SED\n");
                    break;
                default:
            }
            cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t jmp_absolute(cpu_t* cpu, Memory_t* memory)
{
    status_t status = get_operand_absolute(cpu, memory);

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            if (status != STATUS_SUCCESS)
            {
                printf("get_operand_absolute failed with %d\n", status);
            }
            else
            {
                cpu->state = CPU_STATE_DECODE;
            }
            break;
        case CPU_STATE_DECODE:
            switch (status)
            {
                case STATUS_SUCCESS:
                    printf("JMP 0x%04x\n", cpu->effective_address);
                    cpu->PC = cpu->effective_address;
                    cpu->state = CPU_STATE_FETCH;
                    break;
                case STATUS_INPROGRESS:
                    status = STATUS_SUCCESS;
                    break;
                default:
                    printf("get_operand_absolute failed with error %d\n", status);
            }
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t sta_absolute(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            status = get_operand_absolute(cpu, memory);
            if (status != STATUS_SUCCESS)
            {
                printf("get_operand_absolute failed with %d\n", status);
            }
            else
            {
                cpu->state = CPU_STATE_DECODE;
            }
            break;
        case CPU_STATE_DECODE:
            status = get_operand_absolute(cpu, memory);
            switch (status)
            {
                case STATUS_SUCCESS:
                    cpu->state = CPU_STATE_EXECUTE;
                    break;
                case STATUS_INPROGRESS:
                    status = STATUS_SUCCESS;
                    break;
                default:
                    printf("get_operand_absolute failed with error %d\n", status);
            }
            break;
        case CPU_STATE_EXECUTE:
            memory_write_byte(memory, cpu->effective_address, cpu->A);
            printf("STA 0x%04x\n", cpu->effective_address);
            cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t lda_immediate(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            cpu->state = CPU_STATE_EXECUTE;
            cpu->PC++;
            break;
        case CPU_STATE_EXECUTE:
            status = memory_read_byte(memory, cpu->PC, &cpu->A);
            if (status != STATUS_SUCCESS)
            {
                printf("memory_read_byte failed with error %d\n", status);
            }
            else
            {
                printf("LDA #0x%02x\n", cpu->A);
                cpu->state = CPU_STATE_FETCH;
                cpu->PC++;
            }
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

static opcode_t opcode_jump_table[] = {
	[0x00] = NULL,
	[0x01] = NULL,
	[0x02] = NULL,
	[0x03] = NULL,
	[0x04] = NULL,
	[0x05] = NULL,
	[0x06] = NULL,
	[0x07] = NULL,
	[0x08] = NULL,
	[0x09] = NULL,
	[0x0A] = NULL,
	[0x0B] = NULL,
	[0x0C] = NULL,
	[0x0D] = NULL,
	[0x0E] = NULL,
	[0x0F] = NULL,
	[0x10] = NULL,
	[0x11] = NULL,
	[0x12] = NULL,
	[0x13] = NULL,
	[0x14] = NULL,
	[0x15] = NULL,
	[0x16] = NULL,
	[0x17] = NULL,
	[0x18] = proc_flag_set_clear,
	[0x19] = NULL,
	[0x1A] = NULL,
	[0x1B] = NULL,
	[0x1C] = NULL,
	[0x1D] = NULL,
	[0x1E] = NULL,
	[0x1F] = NULL,
	[0x20] = NULL,
	[0x21] = NULL,
	[0x22] = NULL,
	[0x23] = NULL,
	[0x24] = NULL,
	[0x25] = NULL,
	[0x26] = NULL,
	[0x27] = NULL,
	[0x28] = NULL,
	[0x29] = NULL,
	[0x2A] = NULL,
	[0x2B] = NULL,
	[0x2C] = NULL,
	[0x2D] = NULL,
	[0x2E] = NULL,
	[0x2F] = NULL,
	[0x30] = NULL,
	[0x31] = NULL,
	[0x32] = NULL,
	[0x33] = NULL,
	[0x34] = NULL,
	[0x35] = NULL,
	[0x36] = NULL,
	[0x37] = NULL,
	[0x38] = proc_flag_set_clear,
	[0x39] = NULL,
	[0x3A] = NULL,
	[0x3B] = NULL,
	[0x3C] = NULL,
	[0x3D] = NULL,
	[0x3E] = NULL,
	[0x3F] = NULL,
	[0x40] = NULL,
	[0x41] = NULL,
	[0x42] = NULL,
	[0x43] = NULL,
	[0x44] = NULL,
	[0x45] = NULL,
	[0x46] = NULL,
	[0x47] = NULL,
	[0x48] = NULL,
	[0x49] = NULL,
	[0x4A] = NULL,
	[0x4B] = NULL,
	[0x4C] = jmp_absolute,
	[0x4D] = NULL,
	[0x4E] = NULL,
	[0x4F] = NULL,
	[0x50] = NULL,
	[0x51] = NULL,
	[0x52] = NULL,
	[0x53] = NULL,
	[0x54] = NULL,
	[0x55] = NULL,
	[0x56] = NULL,
	[0x57] = NULL,
	[0x58] = proc_flag_set_clear,
	[0x59] = NULL,
	[0x5A] = NULL,
	[0x5B] = NULL,
	[0x5C] = NULL,
	[0x5D] = NULL,
	[0x5E] = NULL,
	[0x5F] = NULL,
	[0x60] = NULL,
	[0x61] = NULL,
	[0x62] = NULL,
	[0x63] = NULL,
	[0x64] = NULL,
	[0x65] = NULL,
	[0x66] = NULL,
	[0x67] = NULL,
	[0x68] = NULL,
	[0x69] = NULL,
	[0x6A] = NULL,
	[0x6B] = NULL,
	[0x6C] = NULL,
	[0x6D] = NULL,
	[0x6E] = NULL,
	[0x6F] = NULL,
	[0x70] = NULL,
	[0x71] = NULL,
	[0x72] = NULL,
	[0x73] = NULL,
	[0x74] = NULL,
	[0x75] = NULL,
	[0x76] = NULL,
	[0x77] = NULL,
	[0x78] = proc_flag_set_clear,
	[0x79] = NULL,
	[0x7A] = NULL,
	[0x7B] = NULL,
	[0x7C] = NULL,
	[0x7D] = NULL,
	[0x7E] = NULL,
	[0x7F] = NULL,
	[0x80] = NULL,
	[0x81] = NULL,
	[0x82] = NULL,
	[0x83] = NULL,
	[0x84] = NULL,
	[0x85] = NULL,
	[0x86] = NULL,
	[0x87] = NULL,
	[0x88] = NULL,
	[0x89] = NULL,
	[0x8A] = NULL,
	[0x8B] = NULL,
	[0x8C] = NULL,
	[0x8D] = sta_absolute,
	[0x8E] = NULL,
	[0x8F] = NULL,
	[0x90] = NULL,
	[0x91] = NULL,
	[0x92] = NULL,
	[0x93] = NULL,
	[0x94] = NULL,
	[0x95] = NULL,
	[0x96] = NULL,
	[0x97] = NULL,
	[0x98] = NULL,
	[0x99] = NULL,
	[0x9A] = NULL,
	[0x9B] = NULL,
	[0x9C] = NULL,
	[0x9D] = NULL,
	[0x9E] = NULL,
	[0x9F] = NULL,
	[0xA0] = NULL,
	[0xA1] = NULL,
	[0xA2] = NULL,
	[0xA3] = NULL,
	[0xA4] = NULL,
	[0xA5] = NULL,
	[0xA6] = NULL,
	[0xA7] = NULL,
	[0xA8] = NULL,
	[0xA9] = lda_immediate,
	[0xAA] = NULL,
	[0xAB] = NULL,
	[0xAC] = NULL,
	[0xAD] = NULL,
	[0xAE] = NULL,
	[0xAF] = NULL,
	[0xB0] = NULL,
	[0xB1] = NULL,
	[0xB2] = NULL,
	[0xB3] = NULL,
	[0xB4] = NULL,
	[0xB5] = NULL,
	[0xB6] = NULL,
	[0xB7] = NULL,
	[0xB8] = proc_flag_set_clear,
	[0xB9] = NULL,
	[0xBA] = NULL,
	[0xBB] = NULL,
	[0xBC] = NULL,
	[0xBD] = NULL,
	[0xBE] = NULL,
	[0xBF] = NULL,
	[0xC0] = NULL,
	[0xC1] = NULL,
	[0xC2] = NULL,
	[0xC3] = NULL,
	[0xC4] = NULL,
	[0xC5] = NULL,
	[0xC6] = NULL,
	[0xC7] = NULL,
	[0xC8] = NULL,
	[0xC9] = NULL,
	[0xCA] = NULL,
	[0xCB] = NULL,
	[0xCC] = NULL,
	[0xCD] = NULL,
	[0xCE] = NULL,
	[0xCF] = NULL,
	[0xD0] = NULL,
    [0xD1] = NULL,
    [0xD2] = NULL,
    [0xD3] = NULL,
    [0xD4] = NULL,
    [0xD5] = NULL,
    [0xD6] = NULL,
    [0xD7] = NULL,
    [0xD8] = proc_flag_set_clear,
    [0xD9] = NULL,
    [0xDA] = NULL,
    [0xDB] = NULL,
    [0xDC] = NULL,
    [0xDD] = NULL,
    [0xDE] = NULL,
    [0xDF] = NULL,
	[0xE0] = NULL,
	[0xE1] = NULL,
	[0xE2] = NULL,
	[0xE3] = NULL,
	[0xE4] = NULL,
	[0xE5] = NULL,
	[0xE6] = NULL,
	[0xE7] = NULL,
	[0xE8] = NULL,
	[0xE9] = NULL,
	[0xEA] = NULL,
	[0xEB] = NULL,
	[0xEC] = NULL,
	[0xED] = NULL,
	[0xEE] = NULL,
	[0xEF] = NULL,
	[0xF0] = NULL,
	[0xF1] = NULL,
	[0xF2] = NULL,
	[0xF3] = NULL,
	[0xF4] = NULL,
	[0xF5] = NULL,
	[0xF6] = NULL,
	[0xF7] = NULL,
	[0xF8] = proc_flag_set_clear,
	[0xF9] = NULL,
	[0xFA] = NULL,
	[0xFB] = NULL,
	[0xFC] = NULL,
	[0xFD] = NULL,
	[0xFE] = NULL,
	[0xFF] = NULL,
};

opcode_t get_opcode_jump_table(uint8_t opcode)
{
    return opcode_jump_table[opcode];
}