#include <stdio.h>

#include "memory.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Incorrect number of arguments\n");
        return -1;
    }

    char* rom_file_path = argv[1];

    Memory_t memory = {};
    memory_init(&memory);
    printf("argc = %d\n", argc);

    memory_load_rom(&memory, rom_file_path);

    printf("argc = %d\n", argc);
}