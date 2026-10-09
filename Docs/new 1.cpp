#define CPU_REGISTER_Y		0x0
#define CPU_REGISTER_A		0x1
#define CPU_REGISTER_X		0x2

#define CPU_ADDR_MODE_INDIRECT_X		0x0
#define CPU_ADDR_MODE_ZEROPAGE			0x1
#define CPU_ADDR_MODE_IMMEDIATE			0x2
#define CPU_ADDR_MODE_ABSOLUTE			0x3
#define CPU_ADDR_MODE_INDIRECT_Y		0x4
#define CPU_ADDR_MODE_ZERO_PAGE_IDX		0x5
#define CPU_ADDR_MODE_ABSOLUTE_Y		0x6
#define CPU_ADDR_MODE_ABSOLUTE_X		0x7


status_t get_operand_immediate(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			break;
		case 1:
			// Read operand byte
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->PC, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
			}
			
			cpu->operand = operand_byte;
			cpu->PC++;
			status = STATUS_SUCCESS;
			break;
		default:
	}
	
    return status;
}

status_t get_operand_zero_page(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			break;
		case 1:
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = zp_addr;
			cpu->PC++;
			break;
		case 2:
			// Obtain operand
			uint8_t zp_addr = (cpu->operand & 0xFF);
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, zp_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->operand = operand_byte;
			status = STATUS_SUCCESS;
			break;
		
		default:
	}
	
    return status;
}

status_t get_operand_zero_page_indexed(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			break;
		case 1:
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = zp_addr;
			cpu->PC++;
			break;
		case 2:
			// Execute dummy read of zero page address
			uint8_t zp_addr = (cpu->operand & 0xFF);
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, zp_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// Obtained zero page address must be incremented by register X or Y based
			// on the instruction
			uint8_t target_register = cpu_get_target_register(uint8_t opcode);
			uint8_t index = (target_register == CPU_REGISTER_Y) ? cpu->Y : cpu->X;	
			cpu->operand = (zp_addr + index) & 0xFF;
			break;
		case 3:
			// Read incremented zero page address and save the data
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			status = STATUS_SUCCESS;
			break;
		
		default:
	}
	
    return status;
}

STA (STore Accumulator)
Zero Page     STA $44       $85  2   3			100 001 01
Zero Page,X   STA $44,X     $95  2   4			100 101 01 
Absolute      STA $4400     $8D  3   4			100 011 01
Absolute,X    STA $4400,X   $9D  3   5			100 111 01
Absolute,Y    STA $4400,Y   $99  3   5			100 110 01
Indirect,X    STA ($44,X)   $81  2   6			100 000 01
Indirect,Y    STA ($44),Y   $91  2   6			100 100 01

STX (STore X register)
Zero Page     STX $44       $86  2   3			100 001 00
Zero Page,Y   STX $44,Y     $96  2   4			100 101 10
Absolute      STX $4400     $8E  3   4			100 011 10

STY (STore Y register)
Zero Page     STY $44       $84  2   3			100 001 00
Zero Page,X   STY $44,X     $94  2   4			100 101 00 
Absolute      STY $4400     $8C  3   4			100 011 00

#define CPU_REGISTER_Y					0x0
#define CPU_REGISTER_A					0x1
#define CPU_REGISTER_X					0x2

#define CPU_ADDR_MODE_INDIRECT_X		0x0
#define CPU_ADDR_MODE_ZEROPAGE			0x1
#define CPU_ADDR_MODE_IMMEDIATE			0x2
#define CPU_ADDR_MODE_ABSOLUTE			0x3
#define CPU_ADDR_MODE_INDIRECT_Y		0x4
#define CPU_ADDR_MODE_ZERO_PAGE_IDX		0x5
#define CPU_ADDR_MODE_ABSOLUTE_Y		0x6
#define CPU_ADDR_MODE_ABSOLUTE_X		0x7


STA (STore Accumulator)
Indirect,X    STA ($44,X)   $81  2   6			100 000 01

Zero Page     STA $44       $85  2   3			100 001 01
Zero Page     STX $44       $86  2   3			100 001 10
Zero Page     STY $44       $84  2   3			100 001 00

Absolute      STA $4400     $8D  3   4			100 011 01
Absolute      STX $4400     $8E  3   4			100 011 10
Absolute      STY $4400     $8C  3   4			100 011 00

Indirect,Y    STA ($44),Y   $91  2   6			100 100 01

Zero Page,X   STA $44,X     $95  2   4			100 101 01
Zero Page,Y   STX $44,Y     $96  2   4			100 101 10
Zero Page,X   STY $44,X     $94  2   4			100 101 00 

Absolute,Y    STA $4400,Y   $99  3   5			100 110 01
 
Absolute,X    STA $4400,X   $9D  3   5			100 111 01


        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  read from the address, add X to it
        4   pointer+X   R  fetch effective address low
        5  pointer+X+1  R  fetch effective address high

status_t get_operand_indirect_X(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	// Maintin indirect address as static variable. Since this only
	// exists over the course of the instruction, keep it as a static
	// var instead of adding new cpu_t field
	static uint16_t operand_addr = 0x0;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			operand_addr = 0;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			operand_addr = zp_addr;
			cpu->PC++;
			break;
		}
		case 2:
		{
			// Dummy read from operand_addr
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, operand_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// Add X to operand_addr (The effective address
			// is always fetched from zero page, so no need to handle
			// page crossing)
			operand_addr = (operand_addr + cpu->X) & 0xFF;
			break;
		}
		case 3:
		{
			// Get low bit of effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, operand_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = operand_byte & 0xFF;
			break;
		}
		case 4:
		{
			// Get high bit of effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, operand_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand |= (operand_byte << 8);
			break;
		}
	}
	
    return status;
}

  Indirect indexed addressing

     Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP)

        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  fetch effective address low
        4   pointer+1   R  fetch effective address high,
                           add Y to low byte of effective address
        5   address+Y*  R  read from effective address,
                           fix high byte of effective address
        6+  address+Y   R  read from effective address

       Notes: The effective address is always fetched from zero page,
              i.e. the zero page boundary crossing is not handled.

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.

              + This cycle will be executed only if the effective address
                was invalid during cycle #5, i.e. page boundary was crossed.


status_t get_operand_indirect_Y(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	// Maintin indirect address as static variable. Since this only
	// exists over the course of the instruction, keep it as a static
	// var instead of adding new cpu_t field
	static uint16_t operand_addr = 0x0;
	static bool page_crossing = false;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			operand_addr = 0;
			page_crossing = false;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			operand_addr = zp_addr;
			cpu->PC++;
			break;
		}
		case 2:
		{
			// Fetch low byte of effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, operand_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = (operand_byte & 0xFF);
			operand_addr++;
			break;
		}
		case 3:
		{
			// Get high byte of effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, operand_addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// Add Y to low byte of effective address. If a page crossing occurs, erase
			// the additional high byte
			cpu->operand += cpu->Y;		
			if (cpu->operand > 0xFF)
			{
				page_crossing = true;
				cpu->operand &= 0xFF;
			}
			
			// Place high byte in effective address
			cpu->operand = (operand_byte << 8);
			break;
		}
		default:
		{
			// Read from effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// If a page fault occured in the previous cycle, fix the high byte
			if (page_crossing)
			{
				// Extract and increment high byte
				uint8_t high_byte = (cpu->operand >> 8) & 0xFF;
				high_byte++;
				
				// Clear high byte
				cpu->operand &= 0xFF;
				
				// Replace high byte
				cpu->operand |= high_byte;
				
				page_crossing = false;
			}
			else
			{
				status = STATUS_SUCCESS;
			}
			break;
		}
	}
	
    return status;
}


