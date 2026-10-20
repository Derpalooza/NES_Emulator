#include "memory.h"
#include "nes_memory_map.h"
#include "bus.h"

// For RAM, mirroring can be implemented by taking the lower 11 bits of the address
#define MIRROR_ADDRESS(addr)    ((uint16_t)((addr) & 0x7FF))

// The RAM only needs to subscribe to reads and writes. It isn't sensitive to clock cycles
static const bus_events_t s_ram_events = {
    .on_clock_cycle = NULL,
    .on_read = memory_read,
    .on_write = memory_write
};

static uint8_t s_RAM[NES_RAM_SIZE] = {0};

status_t memory_init(void)
{
    status_t status = bus_register_device(BUS_DEVICE_RAM, &s_ram_events);
    if (status != STATUS_SUCCESS)
    {
        printf("bus_register_device failed with status = %d", status);
        return status;
    }

    memset(s_RAM, 0x0, NES_RAM_SIZE);

    return STATUS_SUCCESS;
}

status_t memory_read(uint16_t addr, uint8_t *data)
{
    if (addr > NES_RAM_MIRROR_END)
    {
        // If the address lies outside the RAM mirror range, then ignore the read
        return STATUS_SUCCESS;
    }

    *data = s_RAM[MIRROR_ADDRESS(addr)];
    return STATUS_SUCCESS;
}

status_t memory_write(uint16_t addr, uint8_t data)
{
    if (addr > NES_RAM_MIRROR_END)
    {
        // If the address lies outside the RAM mirror range, then ignore the read
        return STATUS_SUCCESS;
    }

    s_RAM[MIRROR_ADDRESS(addr)] = data;
    return STATUS_SUCCESS;
}
