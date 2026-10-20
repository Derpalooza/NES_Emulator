#ifndef _NES_MEMORY_H_
#define _NES_MEMORY_H_

#include <stdint.h>

/* ============================================================
 * NES 6502 CPU Memory Map
 * CPU address space: $0000-$FFFF (64 KB)
 * ============================================================ */

/* Internal RAM: 2 KB, mirrored through $1FFF */
#define NES_RAM_START               0x0000
#define NES_RAM_END                 0x07FF
#define NES_RAM_SIZE                0x0800

#define NES_RAM_MIRROR_START        0x0800
#define NES_RAM_MIRROR_END          0x1FFF

/* PPU registers: mirrored every 8 bytes */
#define NES_PPU_REG_START           0x2000
#define NES_PPU_REG_END             0x2007

#define NES_PPUCTRL                 0x2000
#define NES_PPUMASK                 0x2001
#define NES_PPUSTATUS               0x2002
#define NES_OAMADDR                 0x2003
#define NES_OAMDATA                 0x2004
#define NES_PPUSCROLL               0x2005
#define NES_PPUADDR                 0x2006
#define NES_PPUDATA                 0x2007

#define NES_PPU_MIRROR_START        0x2008
#define NES_PPU_MIRROR_END          0x3FFF
#define NES_PPU_MIRROR_SIZE         0x4000

/* APU and I/O registers */
#define NES_APU_IO_START            0x4000
#define NES_APU_IO_END              0x401F

#define NES_APU_PULSE1_START        0x4000
#define NES_APU_PULSE2_START        0x4004
#define NES_APU_TRIANGLE_START      0x4008
#define NES_APU_NOISE_START         0x400C
#define NES_APU_DMC_START           0x4010

#define NES_OAMDMA                  0x4014
#define NES_APUSTATUS               0x4015
#define NES_CONTROLLER1             0x4016
#define NES_CONTROLLER2             0x4017

/* Cartridge expansion space */
#define NES_EXPANSION_START         0x4020
#define NES_EXPANSION_END           0x5FFF

/* Cartridge PRG-RAM / save RAM (mapper-dependent) */
#define NES_PRG_RAM_START           0x6000
#define NES_PRG_RAM_END             0x7FFF
#define NES_PRG_RAM_SIZE            0x2000

/* Cartridge program ROM / banked program space */
#define NES_PRG_ROM_START           0x8000
#define NES_PRG_ROM_END             0xFFFF
#define NES_PRG_ROM_SIZE            0x8000

#define NES_PRG_ROM_LOWER_START     0x8000
#define NES_PRG_ROM_LOWER_END       0xC000

#define NES_PRG_ROM_UPPER_START     0xC000
#define NES_PRG_ROM_UPPER_END       0x10000

#define NES_PRG_ROM_BANK_SIZE       0x4000

/* 6502 interrupt vectors */
#define NES_NMI_VECTOR              0xFFFA
#define NES_RESET_VECTOR            0xFFFC
#define NES_IRQ_BRK_VECTOR          0xFFFE

/* Address-space limits */
#define NES_CPU_ADDR_MIN            0x0000
#define NES_CPU_ADDR_MAX            0xFFFF
#define NES_CPU_ADDR_SPACE          0x10000

#endif