#ifndef _OPCODE_H_
#define _OPCODE_H_

#include <stdint.h>

#include "cpu_internal.h"

typedef status_t (*opcode_t)(cpu_t*);

opcode_t get_opcode_jump_table(uint8_t opcode);

#endif