#ifndef _CPU_H_
#define _CPU_H_

#include "status.h"

#include <stdint.h>

// Put CPU to initial state
status_t cpu_init();

// Advance the CPU by one cycle
status_t cpu_execute_cycle();

#endif