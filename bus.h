#ifndef _BUS_H_
#define _BUS_H_

#include <stdint.h>

#include "status.h"

// Zero initialize the NES memory
status_t bus_init();

// Load a ROM into memory
status_t bus_load_rom(char* rom_file_path);

// Read a byte from the specified address
status_t bus_read_byte(uint16_t addr, uint8_t* data);

// Write a byte to the specified address
status_t bus_write_byte(uint16_t addr, uint8_t data);

#endif