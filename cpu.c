#include "cpu.h"

#include <stdio.h>

#include "opcode.h"
#include "memory.h"

// TODO: The ROM start address will be read directly from the reset vector for now. But this needs to change
//       once interrupts are properly implemented
#define CPU_RESET_VECTOR                   0xFFFC
#define STACK_POINTER_RESET                0x00

status_t cpu_init(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

    // Read start address
    uint8_t start_addr_lower = 0;
    uint8_t start_addr_upper = 0;

    status = memory_read_byte(memory, CPU_RESET_VECTOR, &start_addr_lower);
    if (status != STATUS_SUCCESS)
    {
        printf("memory_read_byte failed with status = %d", status);
        return status;
    }

    status = memory_read_byte(memory, CPU_RESET_VECTOR+1, &start_addr_upper);
    if (status != STATUS_SUCCESS)
    {
        printf("memory_read_byte failed with status = %d", status);
        return status;
    }


    cpu->PC = (start_addr_upper << 8) | start_addr_lower;
    printf("Reset address == 0x%x\n", cpu->PC);

    cpu->SP = STACK_POINTER_RESET;

    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->P = STATUS_UNUSED_MASK;

    cpu->state = CPU_STATE_FETCH;
    cpu->cycle_count = 0;

    cpu->current_opcode_cycles = 0;
    cpu->operand = 0;
}

status_t cpu_trigger_interrupt(cpu_t* cpu, Memory_t* memory)
{
    
}

// Advance the CPU by one cycle
status_t cpu_execute_cycle(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

    if (cpu->state == CPU_STATE_FETCH)
    {
        // Grab and save the next opcode
        status = memory_read_byte(memory, cpu->PC, &cpu->current_opcode);
        if (status != STATUS_SUCCESS)
        {
            printf("memory_read_byte failed with status = %d\n", status);
            return status;
        }

        cpu->current_opcode_cycles = 0;
    }

    printf("PC = 0x%04x, opcode = 0x%02x, status = 0x%02x, X = 0x%02x, Y = 0x%02x, A = 0x%02x, stack = 0x%02x, state = %d\n", cpu->PC, cpu->current_opcode, cpu->P, cpu->X, cpu->Y, cpu->A, cpu->SP, cpu->state);

    opcode_t opcode_fn = get_opcode_jump_table(cpu->current_opcode);
    if (opcode_fn == NULL)
    {
        printf("Invalid opcode 0x%02x\n", cpu->current_opcode);
        return STATUS_EINVALIDPARAMS;
    }
    else
    {
        status = opcode_fn(cpu, memory);
        if (status != STATUS_SUCCESS)
        {
            printf("Running opcode failed with status = %d\n", status);
            return status;
        }
    }

    
    cpu->cycle_count++;
    cpu->current_opcode_cycles++;

    if (cpu->state == CPU_STATE_FETCH)
    {
        printf("Took %d cycles\n\n");
    }
    return 0;
}