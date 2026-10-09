#ifndef _CPU_H_
#define _CPU_H_

#include "memory.h"
#include "status.h"

// Processor Status register masks
#define STATUS_CARRY_FLAG_MASK              (1 << 0)
#define STATUS_ZERO_FLAG_MASK               (1 << 1)
#define STATUS_INTERRUPT_DISABLE_MASK       (1 << 2)
#define STATUS_DECIMAL_MODE_MASK            (1 << 3)
#define STATUS_UNUSED_MASK                  (1 << 4)
#define STATUS_OVERFLOW_MASK                (1 << 6)
#define STATUS_NEGATIVE_MASK                (1 << 7)

typedef enum
{
    CPU_STATE_FETCH = 0,            // Waiting to fetch new instruction
    CPU_STATE_DECODE,               // Decoding remianing operands
    CPU_STATE_EXECUTE,              // Executing instruction logic
    NUM_CPU_STATES
} cpu_state_t;

typedef struct
{
    // Registers
    uint16_t    PC;     // Program Counter
    uint8_t     SP;     // Stack Pointer
    uint8_t     A;      // Accumulator Register
    uint8_t     X;      // Index Regster X
    uint8_t     Y;      // Index Register Y
    uint8_t     P;      // Processor Status Register

    // Tracking data
    uint32_t    cycle_count;
    uint32_t    state;                  // Current point in the opcode state machine
    uint8_t     current_opcode;         // Holds the opcode currently being processed
    uint8_t     current_opcode_cycles;  // Number of cycles that were spent processing the current opcode
    uint16_t    operand;                // Store operand obtained from opcode
} cpu_t;

// Put CPU to initial state
status_t cpu_init(cpu_t* cpu, Memory_t* memory);

status_t cpu_trigger_interrupt(cpu_t* cpu, Memory_t* memory);

// Advance the CPU by one cycle
status_t cpu_execute_cycle(cpu_t* cpu, Memory_t* memory);


#endif