#include "bus.h"

#include <stdio.h>
#include <string.h>

#define RAM_SIZE            0x800
#define NUM_PPU_REGISTERS   8
#define NUM_APU_REGISTERS   0x20
#define EXPANSION_ROM_SIZE  0x2000
#define SRAM_SIZE           0x2000
#define PRG_ROM_SIZE        0x4000

#define INES_MAGIC_NUMBER   0x1A53454E

typedef struct 
{
    uint32_t    magic_number;
    uint8_t     prg_rom_size;
    uint8_t     chr_rom_size;
    uint8_t     ctrl1;
    uint8_t     ctrl2;
    uint8_t     padding[7];     // Last 7 bytes of the header are reserved for future upgrades
} INES_Header_t;

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

// Global memory instance
static Memory_t g_memory = {0};

// Parse 16-bit address and return a pointer to the corresponding data
static uint8_t* route_address(uint16_t addr)
{
    uint8_t *target_addr;

    // TODO: Create a header file defining each of these address ranges so we don't have to use
    //       magic numbers to decide the ranges
    if (addr < 0x2000)
    {
        // The address lies in RAM. Use the lower 11 bits to simulate mirroring
        target_addr = &g_memory.RAM[addr & 0x7FF];
    }
    else if (addr < 0x4000)
    {
        // The address targets the PPU I/O registers. Use lower 8 bits to simulate mirroring
        target_addr = &g_memory.PPU_Registers[addr & 0x7];
    }
    else if (addr < 0x4020)
    {
        // The address targets the APU I/O registers
        target_addr = &g_memory.APU_Registers[addr & 0x1F];
    }
    else if (addr < 0x6000)
    {
        // The address lies in the Expansion ROM
        target_addr = &g_memory.Expansion_ROM[addr & 0x1FFF];
    }
    else if (addr < 0x8000)
    {
        // The address lies in SRAM
        target_addr = &g_memory.SRAM[addr & 0x1FFF];
    }
    else if (addr < 0xC000)
    {
        // The address lies in the PRG-ROM lower bank
        target_addr = &g_memory.PRG_ROM_Lower_Bank[addr & 0x3FFF];
    }
    else
    {
        // The address lies in the PRG-ROM upper bank
        target_addr = &g_memory.PRG_ROM_Upper_Bank[addr & 0x3FFF];
    }

    return target_addr;
}

status_t bus_init()
{
    memset(&g_memory, 0, sizeof(Memory_t));

    return STATUS_SUCCESS;
}

status_t bus_load_rom(char* rom_file_path)
{
    if (rom_file_path == NULL)
    {
        printf("Invalid parameters/file path\n");
        return STATUS_EINVALIDPARAMS;
    }

    FILE *file = fopen(rom_file_path, "rb");
    if (file == NULL) {
        printf("Error: Unable to open %s\n", rom_file_path);
        return STATUS_EFILEERROR;
    }

    // Read iNES header
    INES_Header_t header = {0};
    size_t bytes_read = fread(&header, 1, sizeof(INES_Header_t), file);
    if (bytes_read != sizeof(INES_Header_t))
    {
        printf("Error: Read %d of bytes from %s ROM header instead of %d\n", bytes_read, rom_file_path, sizeof(INES_Header_t));
        return STATUS_EFILEERROR;
    }

    if (INES_MAGIC_NUMBER != header.magic_number)
    {
        printf("Error: %s is not a valid iNES ROM\n", rom_file_path);
        return STATUS_EFILEERROR;
    }

    printf("Magic number = 0x%x\n", header.magic_number);
    printf("PRG ROM size = %d\n", header.prg_rom_size);
    printf("CHR ROM size = %d\n", header.chr_rom_size);

    // TODO: Until mappers are supported, default to mapper 0 behavior
    uint8_t mapper_number = (header.ctrl1 & 0xF0) | ((header.ctrl1 >> 4) & 0xF);
    printf("Mapper number = %d\n", mapper_number);

    if (mapper_number != 0)
    {
        printf("Error: Mapper numbers other than 0 are currently unsupported");
        return STATUS_EINVALIDPARAMS;
    }
    else if ((header.prg_rom_size == 0) || (header.prg_rom_size > 2))
    {
        printf("Error: Invalid number of PRG-ROM banks (%d banks)", header.prg_rom_size);
        return STATUS_EINVALIDPARAMS;
    }

    // Load PRG-ROM segments into lower bank.
    bytes_read = fread(g_memory.PRG_ROM_Lower_Bank, 1, PRG_ROM_SIZE, file);
    if (bytes_read != PRG_ROM_SIZE)
    {
        printf("Error: Read %d of bytes instead of %d\n", bytes_read, PRG_ROM_SIZE);
        return STATUS_EFILEERROR;
    }

    if (header.prg_rom_size == 2)
    {
        // If there is another segment, load it into the upper bank
        bytes_read = fread(g_memory.PRG_ROM_Upper_Bank, 1, PRG_ROM_SIZE, file);
        if (bytes_read != PRG_ROM_SIZE)
        {
            printf("Error: Read %d of bytes instead of %d\n", bytes_read, PRG_ROM_SIZE);
            return STATUS_EFILEERROR;
        }
    }
    else
    {
        // Copy lower bank into upper bank
        memcpy(g_memory.PRG_ROM_Upper_Bank, g_memory.PRG_ROM_Lower_Bank, PRG_ROM_SIZE);
    }

    fclose(file);
}

status_t bus_read_byte(uint16_t addr, uint8_t* data)
{
    uint8_t *target_addr = route_address(addr);

    *data = *target_addr;

    return STATUS_SUCCESS;
}

status_t bus_write_byte(uint16_t addr, uint8_t data)
{
    uint8_t *target_addr = route_address(addr);

    *target_addr = data;

    return STATUS_SUCCESS;
}