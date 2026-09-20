#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

#include "status.h"

#define RAM_SIZE            0x800
#define NUM_PPU_REGISTERS   8
#define NUM_APU_REGISTERS   0x20
#define EXPANSION_ROM_SIZE  0x2000
#define SRAM_SIZE           0x2000
#define PRG_ROM_SIZE        0x4000

typedef struct
{
    uint8_t RAM[RAM_SIZE];
    uint8_t PPU_Registers[NUM_PPU_REGISTERS];
    uint8_t APU_Registers[NUM_APU_REGISTERS];
    uint8_t Expansion_ROM[EXPANSION_ROM_SIZE];
    uint8_t SRAM[SRAM_SIZE];
    uint8_t PRG_ROM_Lower_Bank[PRG_ROM_SIZE];
    uint8_t PRG_ROM_Upper_Bank[PRG_ROM_SIZE];
} Memory_t;

// Zero initialize the NES memory
status_t memory_init(Memory_t* memory);

// Load a ROM into memory
status_t memory_load_rom(Memory_t* memory, char* rom_file_path);

// Read a byte from the specified address
status_t memory_read_byte(Memory_t* memory, uint16_t addr, uint8_t* data);

// Write a byte to the specified address
status_t memory_write_byte(Memory_t* memory, uint16_t addr, uint8_t data);

#endif