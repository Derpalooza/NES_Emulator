#ifndef _CPU_INTERNAL_H_
#define _CPU_INTERNAL_H_

#include <stdint.h>

#include "status.h"

// Internal CPU helper functions

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

// TODO: The DECODE state doesn't actually do any decoding. We just fetch the effective addresses from the
//       instruciton operand, making it really a fetch operation. Our actual FETCH state functions more like an
//       idle state. Rename DECODE->FETCH and FETCH->IDLE
typedef enum
{
    CPU_STATE_FETCH = 0,            // CPU is idle and Waiting to fetch new instruction
    CPU_STATE_DECODE,               // CPU is currently fetching operand addresses
    CPU_STATE_READ,                 // CPU is reading operand
    CPU_STATE_EXECUTE,              // CPU is executing instruction logic (Represents modify stage for read-modify-write instructions)
    CPU_STATE_WRITE,                // CPU is writing to memory
    NUM_CPU_STATES
} cpu_state_t;

typedef union
{
    uint16_t addr;
    struct
    {
        uint8_t low_byte;
        uint8_t high_byte;
    } bytes;
} cpu_addr_t;

typedef struct
{
    // Registers
    cpu_addr_t  PC;     // Program Counter
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
    cpu_addr_t  effective_address;      // Address of the opcode operand
    uint16_t    operand;                // Store operand obtained from opcode
} cpu_t;

// Populates source register with contents of destination register, and sets status bits
// Registers MUST be set through this function. They must not be modified directly
status_t cpu_set_register(cpu_t* cpu, cpu_register_t reg, uint8_t src);

// Arithmetic functions. Will also handle status bits internally
uint8_t cpu_add_data(cpu_t* cpu, uint8_t A, uint8_t B);

// Obtain the addressing mode of the given opcode and returns a CPU_ADDR_MODE_* macro
cpu_addr_mode_t cpu_get_opcode_addressing_mode(uint8_t opcode);

status_t cpu_get_opcode_register(uint8_t opcode, cpu_register_t* reg);

// Obtain the target register of the given opcode and returns a CPU_REGISTER_* macro
uint8_t cpu_get_target_register(uint8_t opcode);

// Print the instruction mnemonic for debugging purposes
void cpu_print_instruction(uint16_t effective_addr, uint8_t operand, char* instr_name, uint8_t addr_mode);

// TODO: Add stack push/pop

#endif