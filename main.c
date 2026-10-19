#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "bus.h"
#include "cpu.h"
#include "status.h"

int main(int argc, char *argv[])
{
    status_t status = STATUS_SUCCESS;
    if (argc != 2)
    {
        printf("Incorrect number of arguments\n");
        return -1;
    }

    char* rom_file_path = argv[1];

    status = bus_init();
    if (status != STATUS_SUCCESS)
    {
        printf("Failed with error %d\n", status);
        return -1;
    }

    printf("argc = %d\n", argc);

    status = bus_load_rom(rom_file_path);
    if (status != STATUS_SUCCESS)
    {
        printf("Failed with error %d\n", status);
        return -1;
    }

    printf("argc = %d\n", argc);

    cpu_t cpu = {0};
    cpu_init(&cpu);

    while (true)
    {
        int status = cpu_execute_cycle(&cpu);

        if (status != 0)
        {
            printf("Failed to execute CPU cycle\n");
            break;
        }
        //usleep(5000);
    }
}