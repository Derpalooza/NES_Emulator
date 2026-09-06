#include "memory.h"

#include <stdio.h>
#include <string.h>

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

int memory_init(Memory_t* memory)
{
    memset(memory, 0, sizeof(Memory_t));
}

int memory_load_rom(Memory_t* memory, char* rom_file_path)
{
    if ((memory == NULL) || (rom_file_path == NULL))
    {
        printf("Invalid parameters/file path\n");
        return -1;
    }

    FILE *file = fopen(rom_file_path, "rb");
    if (file == NULL) {
        printf("Error: Unable to open %s\n", rom_file_path);
        return -1;
    }

    // Read iNES header
    INES_Header_t header = {};
    size_t bytes_read = fread(&header, 1, sizeof(INES_Header_t), file);
    if (bytes_read != sizeof(INES_Header_t))
    {
        printf("Error: Read %d of bytes from %s ROM header instead of %d\n", bytes_read, rom_file_path, sizeof(INES_Header_t));
        return -1;
    }

    if (INES_MAGIC_NUMBER != header.magic_number)
    {
        printf("Error: %s is not a valid iNES ROM\n", rom_file_path);
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
        return -1;
    }
    else if ((header.prg_rom_size == 0) || (header.prg_rom_size > 2))
    {
        printf("Error: Invalid number of PRG-ROM banks (%d banks)", header.prg_rom_size);
        return -1;
    }

    // Load PRG-ROM segments into lower bank.
    bytes_read = fread(memory->PRG_ROM_Lower_Bank, 1, PRG_ROM_SIZE, file);
    if (bytes_read != PRG_ROM_SIZE)
    {
        printf("Error: Read %d of bytes instead of %d\n", bytes_read, PRG_ROM_SIZE);
        return -1;
    }

    if (header.prg_rom_size == 2)
    {
        // If there is another segment, load it into the upper bank
        bytes_read = fread(memory->PRG_ROM_Upper_Bank, 1, PRG_ROM_SIZE, file);
        if (bytes_read != PRG_ROM_SIZE)
        {
            printf("Error: Read %d of bytes instead of %d\n", bytes_read, PRG_ROM_SIZE);
            return -1;
        }
    }
    else
    {
        // Copy lower bank into upper bank
        memcpy(memory->PRG_ROM_Upper_Bank, memory->PRG_ROM_Lower_Bank, PRG_ROM_SIZE);
    }
}

int memory_read_byte(Memory_t* memory, uint16_t addr, uint8_t* data)
{

}

int memory_write_byte(Memory_t* memory, uint16_t addr, uint8_t data)
{

}