#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

#include "status.h"

status_t memory_init(void);

// Bus event implementations
status_t memory_read(uint16_t addr, uint8_t *data);
status_t memory_write(uint16_t addr, uint8_t data);

#endif