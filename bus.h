#ifndef _BUS_H_
#define _BUS_H_

// Bus implementation for NES system

#include <stdint.h>

#include "status.h"

typedef struct {
    status_t (*on_clock_cycle)(void);
    status_t (*on_read)(uint16_t addr, uint8_t *data);
    status_t (*on_write)(uint16_t addr, uint8_t *data);
} bus_events_t;

typedef enum
{
    BUS_DEVICE_CPU = 0,
    BUS_DEVICE_PPU,
    NUM_BUS_DEVICES,
} bus_device_t;

// Initialize the bus before operation
status_t bus_init();

// Load a ROM into memory
status_t bus_load_rom(char* rom_file_path);

// Devices mapped to the NES are sensitive to reads, writes, and clock cycles. Each device registered to the
// bus must provide implementations for the events it wants to subscribe to
status_t bus_register_device(bus_device_t device_type, const bus_events_t *callbacks);

// Read a byte from the specified address
status_t bus_read_byte(uint16_t addr, uint8_t* data);

// Write a byte to the specified address
status_t bus_write_byte(uint16_t addr, uint8_t data);

// Advance the system by one clock cycle and drive all devices currently subscribing to the clock
status_t bus_execute_clock_cycle();

#endif