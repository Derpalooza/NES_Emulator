#include "cpu.h"

#include <stdio.h>

#include "cpu_internal.h"

#include "opcode.h"
#include "bus.h"

// TODO: The ROM start address will be read directly from the reset vector for now. But this needs to change
//       once interrupts are properly implemented
#define CPU_RESET_VECTOR                   0xFFFC
#define STACK_POINTER_RESET                0x00

// The CPU initiates reads and writes, but isn't sensitive to them. We only need to subscribe to the clock 
static const bus_events_t g_cpu_events = {
    .on_clock_cycle = cpu_execute_cycle,
    .on_read = NULL,
    .on_write = NULL
};

static cpu_t g_cpu = {0};

status_t cpu_init()
{
    status_t status = STATUS_SUCCESS;

    g_cpu = (cpu_t){0};

    // Read start address
    uint8_t start_addr_lower = 0;
    uint8_t start_addr_upper = 0;

    status = bus_read_byte(CPU_RESET_VECTOR, &start_addr_lower);
    if (status != STATUS_SUCCESS)
    {
        printf("bus_read_byte failed with status = %d", status);
        return status;
    }

    status = bus_read_byte(CPU_RESET_VECTOR+1, &start_addr_upper);
    if (status != STATUS_SUCCESS)
    {
        printf("bus_read_byte failed with status = %d", status);
        return status;
    }

    status = bus_register_device(BUS_DEVICE_CPU, &g_cpu_events);
    if (status != STATUS_SUCCESS)
    {
        printf("bus_register_device failed with status = %d", status);
        return status;
    }

    g_cpu.PC.bytes.high_byte = start_addr_upper;
    g_cpu.PC.bytes.low_byte = start_addr_lower;

    printf("Reset address == 0x%x\n", g_cpu.PC);

    g_cpu.SP = STACK_POINTER_RESET;

    g_cpu.A = 0;
    g_cpu.X = 0;
    g_cpu.Y = 0;
    g_cpu.P = STATUS_UNUSED_MASK;

    g_cpu.state = CPU_STATE_FETCH;
    g_cpu.cycle_count = 0;

    g_cpu.current_opcode_cycles = 0;
    g_cpu.operand = 0;
}

// Advance the CPU by one cycle
status_t cpu_execute_cycle()
{
    status_t status = STATUS_SUCCESS;

    if (g_cpu.state == CPU_STATE_FETCH)
    {
        // Grab and save the next opcode
        status = bus_read_byte(g_cpu.PC.addr, &g_cpu.current_opcode);
        if (status != STATUS_SUCCESS)
        {
            printf("bus_read_byte failed with status = %d\n", status);
            return status;
        }

        g_cpu.current_opcode_cycles = 0;
    }

    printf("PC = 0x%04x, opcode = 0x%02x, status = 0x%02x, X = 0x%02x, Y = 0x%02x, A = 0x%02x, stack = 0x%02x, state = %d\n", g_cpu.PC, g_cpu.current_opcode, g_cpu.P, g_cpu.X, g_cpu.Y, g_cpu.A, g_cpu.SP, g_cpu.state);

    opcode_t opcode_fn = get_opcode_jump_table(g_cpu.current_opcode);
    if (opcode_fn == NULL)
    {
        printf("Invalid opcode 0x%02x\n", g_cpu.current_opcode);
        return STATUS_EINVALIDPARAMS;
    }
    else
    {
        status = opcode_fn(&g_cpu);
        if (status != STATUS_SUCCESS)
        {
            printf("Running opcode failed with status = %d\n", status);
            return status;
        }
    }

    
    g_cpu.cycle_count++;
    g_cpu.current_opcode_cycles++;

    if (g_cpu.state == CPU_STATE_FETCH)
    {
        printf("Took %d cycles\n\n", g_cpu.current_opcode_cycles);
    }
    return 0;
}