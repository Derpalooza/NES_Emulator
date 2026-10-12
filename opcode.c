#include "opcode.h"

#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "memory.h"
#include "cpu_internal.h"

status_t get_operand_implied(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;

	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			status = STATUS_INPROGRESS;
			break;
		case 1:
			status = STATUS_SUCCESS;
			break;
	}

    return status;
}

status_t get_operand_immediate(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_INPROGRESS;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
		{
			cpu->PC++;
			break;
		}
		case 1:
		{
			// Read operand byte
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->PC, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = operand_byte;
			cpu->PC++;
			status = STATUS_SUCCESS;
			break;
		}
		default:
			break;
	}
	
    return status;
}

status_t get_operand_absolute(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_INPROGRESS;
	uint8_t operand_byte = 0;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			status = STATUS_INPROGRESS;
			break;
		case 1:
			// Read lower byte of operand
			status = memory_read_byte(memory, cpu->PC, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->operand = operand_byte;
			cpu->PC++;
			status = STATUS_INPROGRESS;
			break;
		case 2:
			// Read upper byte of operand
			status = memory_read_byte(memory, cpu->PC, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->operand |= (operand_byte << 8);
			cpu->PC++;
			status = STATUS_SUCCESS;
			break;
		default:
			printf("Exceeded number of cycles (%d cycles) for this addressing mode\n", cpu->current_opcode_cycles);
			status = STATUS_EINVALIDPARAMS;
			break;
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
		{
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
		}
		case 2:
		{
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
		}
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
		{
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
		}
		case 2:
		{
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
			uint8_t target_register = cpu_get_target_register(cpu->current_opcode);
			uint8_t index = (target_register == CPU_REGISTER_Y) ? cpu->Y : cpu->X;	
			cpu->operand = (zp_addr + index) & 0xFF;
			break;
		}
		case 3:
		{
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
		}
	}
	
    return status;
}

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
			status = STATUS_SUCCESS;
			break;
		}
	}
	
    return status;
}

status_t get_operand_indirect_Y(cpu_t* cpu, Memory_t* memory)
{
	// TODO: Still not ready. Takes wrong amount of cycles
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

status_t get_operand(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	uint8_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);
	
	switch (addr_mode)
	{
		case CPU_ADDR_MODE_IMMEDIATE:
			status = get_operand_immediate(cpu, memory);
			break;
		case CPU_ADDR_MODE_ABSOLUTE:
			status = get_operand_absolute(cpu, memory);
			break;
		case CPU_ADDR_MODE_ZEROPAGE:
			status = get_operand_zero_page(cpu, memory);
			break;
		case CPU_ADDR_MODE_ZERO_PAGE_IDX:
			status = get_operand_zero_page_indexed(cpu, memory);
			break;
		case CPU_ADDR_MODE_INDIRECT_X:
			status = get_operand_indirect_X(cpu, memory);
			break;
		case CPU_ADDR_MODE_INDIRECT_Y:
			status = get_operand_indirect_Y(cpu, memory);
			break;
		case CPU_ADDR_MODE_ABSOLUTE_Y:
		case CPU_ADDR_MODE_ABSOLUTE_X:
			printf("Addressing mode 0x%x not implemented yet\n", addr_mode);
			return STATUS_EINVALIDPARAMS;
			break;
	}
}

// Flag (Processor Status) Instructions
status_t proc_flag_set_clear(cpu_t* cpu, Memory_t* memory)
{
    status_t status = get_operand_implied(cpu, memory);

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
			if (status != STATUS_INPROGRESS)
            {
                printf("get_operand_implied failed with %d\n", status);
            }
            else
            {
                cpu->state = CPU_STATE_EXECUTE;
				status = STATUS_SUCCESS;
            }
            break;
        case CPU_STATE_EXECUTE:
            switch (cpu->current_opcode)
            {
                case 0x18:
                    cpu->P &= ~STATUS_CARRY_FLAG_MASK;
					cpu_print_instruction(cpu->operand, "CLC", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0x38:
                    cpu->P |= STATUS_CARRY_FLAG_MASK;
					cpu_print_instruction(cpu->operand, "CLC", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0x58:
                    cpu->P &= ~STATUS_INTERRUPT_DISABLE_MASK;
					cpu_print_instruction(cpu->operand, "CLI", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0x78:
                    cpu->P |= STATUS_INTERRUPT_DISABLE_MASK;
					cpu_print_instruction(cpu->operand, "SEI", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0xB8:
                    cpu->P &= ~STATUS_OVERFLOW_MASK;
					cpu_print_instruction(cpu->operand, "CLV", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0xD8:
                    cpu->P &= ~STATUS_DECIMAL_MODE_MASK;
					cpu_print_instruction(cpu->operand, "CLD", CPU_ADDR_MODE_IMPLIED);
                    break;
                case 0xF8:
                    cpu->P |= STATUS_DECIMAL_MODE_MASK;
					cpu_print_instruction(cpu->operand, "SED", CPU_ADDR_MODE_IMPLIED);
                    break;
                default:
					printf("Opcode 0x%x is invalid for proc_flag_set_clear\n", cpu->current_opcode);
					status = STATUS_EINVALIDPARAMS;
					break;
            }
            cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t jmp_absolute(cpu_t* cpu, Memory_t* memory)
{
    status_t status = get_operand_absolute(cpu, memory);

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            if (status != STATUS_INPROGRESS)
            {
                printf("get_operand_absolute failed with %d\n", status);
            }
            else
            {
                cpu->state = CPU_STATE_DECODE;
				status = STATUS_SUCCESS;
            }
            break;
        case CPU_STATE_DECODE:
            switch (status)
            {
                case STATUS_SUCCESS:
					cpu_print_instruction(cpu->operand, "JMP", CPU_ADDR_MODE_ABSOLUTE);
                    cpu->PC = cpu->operand;
                    cpu->state = CPU_STATE_FETCH;
                    break;
                case STATUS_INPROGRESS:
                    status = STATUS_SUCCESS;
                    break;
                default:
                    printf("get_operand_absolute failed with error %d\n", status);
            }
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

// TODO: For Store, Indirect Y must take strictly 6 cycles, not just 5+ like everything else. Fix this
status_t store(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;
	
	// Target register is contianed in lower 3 bytes of opcode (Y = 00, X = 10, A = 01)
	uint8_t target_register = cpu->current_opcode & 0x3;
	uint8_t* reg = NULL;
	switch (target_register)
	{
		case CPU_REGISTER_Y:
			reg = &cpu->Y;
			break;
		case CPU_REGISTER_A:
			reg = &cpu->A;
			break;
		case CPU_REGISTER_X:
			reg = &cpu->X;
			break;
		default:
			printf("Invalid register value\n");
			return STATUS_EINVALIDPARAMS;
	}

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            status = get_operand(cpu, memory);
            if (status != STATUS_INPROGRESS)
            {
                printf("get_operand failed with %d\n", status);
            }
            else
            {
                cpu->state = CPU_STATE_DECODE;
				status = STATUS_SUCCESS;
            }
            break;
        case CPU_STATE_DECODE:
            status = get_operand(cpu, memory);
            switch (status)
            {
                case STATUS_SUCCESS:
                    cpu->state = CPU_STATE_EXECUTE;
                    break;
                case STATUS_INPROGRESS:
                    status = STATUS_SUCCESS;
                    break;
                default:
                    printf("get_operand failed with error %d\n", status);
            }
            break;
        case CPU_STATE_EXECUTE:
            memory_write_byte(memory, cpu->operand, *reg);

			// Print opcode information
			char instr_name[8] = {};
			sprintf(instr_name, "ST%s", (target_register == CPU_REGISTER_Y) ? "Y" : (target_register == CPU_REGISTER_A) ? "A" : "X", cpu->operand);
			uint8_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);
			cpu_print_instruction(cpu->operand, instr_name, addr_mode);


            cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t ld_immediate(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

	// Target register is contianed in lower 3 bytes of opcode (Y = 00, X = 10, A = 01)
	uint8_t target_register = cpu->current_opcode & 0x3;
	uint8_t* reg = NULL;
	switch (target_register)
	{
		case 0:
			reg = &cpu->Y;
			break;
		case 1:
			reg = &cpu->A;
			break;
		case 2:
			reg = &cpu->X;
			break;
		default:
			printf("Invalid register value\n");
			return STATUS_EINVALIDPARAMS;
	}

	status = get_operand_immediate(cpu, memory);

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
			if (status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_EXECUTE;
				status = STATUS_SUCCESS;
			}
            break;
        case CPU_STATE_EXECUTE:
			if (status == STATUS_SUCCESS)
			{
				cpu_set_register(cpu, reg, cpu->operand);
				printf("LD%s #0x%02x\n\n", (target_register == 0) ? "Y" : (target_register == 1) ? "A" : "X", *reg);
                cpu->state = CPU_STATE_FETCH;
			}
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t transfer_register(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
			cpu->state = CPU_STATE_EXECUTE;
			cpu->PC++;
            break;
        case CPU_STATE_EXECUTE:
			switch (cpu->current_opcode)
			{
				case 0xA8:
					// Transfer Accumulator to Y
					cpu_set_register(cpu, &cpu->Y, cpu->A);
					cpu_print_instruction(cpu->operand, "TAY", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xAA:
					// Transfer Accumulator to X
					cpu_set_register(cpu, &cpu->X, cpu->A);
					cpu_print_instruction(cpu->operand, "TAX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xBA:
					// Transfer Stack pointer to X
					cpu_set_register(cpu, &cpu->X, cpu->SP);
					cpu_print_instruction(cpu->operand, "TSX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x98:
					// Transfer Y to Accumulator
					cpu_set_register(cpu, &cpu->A, cpu->Y);
					cpu_print_instruction(cpu->operand, "TYA", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x8A:
					// Transfer X to Accumulator
					cpu_set_register(cpu, &cpu->A, cpu->X);
					cpu_print_instruction(cpu->operand, "TXA", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x9A:
					// Transfer X to Stack pointer
					cpu_set_register(cpu, &cpu->SP, cpu->X);
					cpu_print_instruction(cpu->operand, "TXS", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xE8:
					// Increment X
					cpu_set_register(cpu, &cpu->X, cpu->X + 1);
					cpu_print_instruction(cpu->operand, "INX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xC8:
					// Increment Y
					cpu_set_register(cpu, &cpu->Y, cpu->Y + 1);
					cpu_print_instruction(cpu->operand, "INY", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xCA:
					// Increment Y
					cpu_set_register(cpu, &cpu->X, cpu->X - 1);
					cpu_print_instruction(cpu->operand, "DEX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x88:
					// Increment Y
					cpu_set_register(cpu, &cpu->Y, cpu->Y - 1);
					cpu_print_instruction(cpu->operand, "DEY", CPU_ADDR_MODE_IMPLIED);
					break;
				default:
					printf("Opcode 0x%02x is invalid for transfer_register\n", cpu->current_opcode);
					return STATUS_EINVALIDPARAMS;
			}
			cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

	return status;
}

status_t jump_to_subroutine(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

	// The standard state machine seen with the other instructions isn't used
	// for JSR. Because it doesn't collect its operands in a straightforward addressing
	// mode but rather spends cycles updating the stack (MCS6500 MICROCOMPUTER FAMILY PROGRAMMING MANUAL in DOCS folder)
	switch (cpu->current_opcode_cycles)
	{
		case 0:
		{
			cpu->state = CPU_STATE_DECODE;
			cpu->operand = 0x0;
			cpu->PC++;
			status = STATUS_SUCCESS;
			break;
		}
		case 1:
		{
			// Fetch low address byte of subroutine address
			uint8_t low_addr_byte = 0;
			status = memory_read_byte(memory, cpu->PC, &low_addr_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_write_byte failed with error %d", status);
				return status;
			}
			cpu->operand = low_addr_byte;
			cpu->PC++;
			break;
		}
		case 2:
		{
			// Origina
			break;
		}
		case 3:
		{
			// Put high byte of PC on the stack and decrement stack pointer
			uint8_t PC_high_byte = (cpu->PC >> 8);
			status = memory_write_byte(memory, cpu->SP, PC_high_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_write_byte failed with error %d", status);
				return status;
			}
			cpu->SP--;
			break;
		}
		case 4:
		{
			// Put low byte of PC on the stack and decrement stack pointer
			uint8_t PC_low_byte = (cpu->PC & 0xFF);
			status = memory_write_byte(memory, cpu->SP, PC_low_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_write_byte failed with error %d", status);
				return status;
			}
			cpu->SP--;
			break;
		}
		case 5:
		{
			// Fetch high address byte of subroutine address
			uint8_t high_addr_byte = 0;
			status = memory_read_byte(memory, cpu->PC, &high_addr_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_write_byte failed with error %d", status);
				return status;
			}
			cpu->operand |= (high_addr_byte << 8);
			cpu->PC = cpu->operand;
			cpu->state = CPU_STATE_FETCH;
			cpu_print_instruction(cpu->operand, "JSR", CPU_ADDR_MODE_ABSOLUTE);
			break;
		}
	}

    return status;
}

status_t get_operand_relative(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_INPROGRESS;
	uint8_t operand_byte = 0;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC++;
			status = STATUS_INPROGRESS;
			break;
		case 1:
			// Read offset byte stored in operand
			status = memory_read_byte(memory, cpu->PC, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->operand = operand_byte;
			cpu->PC++;
			status = STATUS_INPROGRESS;
			break;
		case 2:
			// Perform dummy read of PC
			status = memory_read_byte(memory, cpu->PC, &operand_byte);

			// Calculate new PC (the branch offset must be signed for relative addressing, as it is a +/- offset)
			int8_t offset = cpu->operand & 0xFF;
			uint16_t new_PC = cpu->PC + offset;

			// Compare the upper bytes of the current and new PC to determine if a page fault occured
			if ((new_PC & 0xFF00) == (cpu->PC & 0xFF00))
			{
				// If the upper bytes are equal, then no page fault happened. We can end the relative addressing operation
				status = STATUS_SUCCESS;
			}
			else
			{
				// If a page fault occurred, save the full new address for the next cycle
				cpu->operand = new_PC;
			}

			// Update lower byte of PC
			cpu->PC = (cpu->PC & 0xFF00) | (new_PC & 0xFF);
			break;
		case 3:
			// Perform dummy read of PC
			status = memory_read_byte(memory, cpu->PC, &operand_byte);

			// Update PC with full new address
			cpu->PC = cpu->operand;

			status = STATUS_SUCCESS;
			break;
		default:
			printf("Exceeded number of cycles (%d cycles) for this addressing mode\n", cpu->current_opcode_cycles);
			status = STATUS_EINVALIDPARAMS;
			break;
	}

    return status;
}

status_t branch(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	status = get_operand_relative(cpu, memory);

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
			if (status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
				status = STATUS_SUCCESS;
			}
			break;
		case CPU_STATE_DECODE:
			if (status == STATUS_INPROGRESS)
			{
				bool branch_taken = false;

				// Check operand against status bits to determine if the branch is taken
				switch (cpu->current_opcode)
				{
					case 0x10:
						// Branch on plus (negative clear)
						if (!(cpu->P & STATUS_NEGATIVE_MASK))
						{
							branch_taken = true;
						}
						break;
					case 0x30:
						// Branch on minus (negative set)
						if (cpu->P & STATUS_NEGATIVE_MASK)
						{
							branch_taken = true;
						}
						break;
					case 0x50:
						// Branch on overflow clear
						if (!(cpu->P & STATUS_OVERFLOW_MASK))
						{
							branch_taken = true;
						}
						break;
					case 0x70:
						// Branch on overflow set
						if (cpu->P & STATUS_OVERFLOW_MASK)
						{
							branch_taken = true;
						}
						break;
					case 0x90:
						// Branch on carry clear
						if (!(cpu->P & STATUS_CARRY_FLAG_MASK))
						{
							branch_taken = true;
						}
						break;
					case 0xB0:
						// Branch on carry set
						if (cpu->P & STATUS_CARRY_FLAG_MASK)
						{
							branch_taken = true;
						}
						break;
					case 0xD0:
						// Branch on not equal (zero clear)
						if (!(cpu->P & STATUS_ZERO_FLAG_MASK))
						{
							branch_taken = true;
						}
						break;
					case 0xF0:
						// Branch on equal (zero set)
						if (cpu->P & STATUS_ZERO_FLAG_MASK)
						{
							branch_taken = true;
						}
						break;
                    default:
                        printf("Opcode 0x%x is invalid for branch", cpu->current_opcode);
                        status = STATUS_EINVALIDPARAMS;
				}

				// If the branch isn't taken, end this instruction by setting the state to FETCH. Otherwise,
				// consume an additional cycle to resolve the branch
				cpu->state = (branch_taken) ? CPU_STATE_EXECUTE : CPU_STATE_FETCH;
				status = STATUS_SUCCESS;
			}
            break;
		case CPU_STATE_EXECUTE:
			// Once the branch has properly resolved, get_operand_relative will return SUCCESS
			// (This lets us stall for any additional cycles that are due to page faulting)
			if (status == STATUS_SUCCESS)
			{
				cpu->state = CPU_STATE_FETCH;
			}

			status = STATUS_SUCCESS;
			break;
		default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
	}

	if (cpu->state == CPU_STATE_FETCH)
	{
		switch (cpu->current_opcode)
		{
			case 0x10:
				cpu_print_instruction(cpu->operand, "BPL", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0x30:
				cpu_print_instruction(cpu->operand, "BMI", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0x50:
				cpu_print_instruction(cpu->operand, "BVC", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0x70:
				cpu_print_instruction(cpu->operand, "BVS", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0x90:
				cpu_print_instruction(cpu->operand, "BCC", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0xB0:
				cpu_print_instruction(cpu->operand, "BCS", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0xD0:
				cpu_print_instruction(cpu->operand, "BNE", CPU_ADDR_MODE_RELATIVE);
				break;
			case 0xF0:
				cpu_print_instruction(cpu->operand, "BEQ", CPU_ADDR_MODE_RELATIVE);
				break;
            default:
                printf("Opcode 0x%x is invalid for branch", cpu->current_opcode);
                status = STATUS_EINVALIDPARAMS;
		}
	}

	return status;
}

status_t increment(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
				cpu->state = CPU_STATE_DECODE;
			break;
		case CPU_STATE_DECODE:
            status = get_operand(cpu, memory);
			if (status == STATUS_SUCCESS)
			{
				cpu->state = CPU_STATE_EXECUTE;
				
			}
            break;
        case CPU_STATE_EXECUTE:
		{
			// This is a read-modify-write instruction. Must perform a dummy write of the operand
			// to the effective address before modifying the operand
			break;
		}
		default:
			printf("Exceeded number of cycles (%d cycles) for this addressing mode\n", cpu->current_opcode_cycles);
			status = STATUS_EINVALIDPARAMS;
			break;
	}
    
    return status;
}

static opcode_t opcode_jump_table[] = {
	[0x00] = NULL,
	[0x01] = NULL,
	[0x02] = NULL,
	[0x03] = NULL,
	[0x04] = NULL,
	[0x05] = NULL,
	[0x06] = NULL,
	[0x07] = NULL,
	[0x08] = NULL,
	[0x09] = NULL,
	[0x0A] = NULL,
	[0x0B] = NULL,
	[0x0C] = NULL,
	[0x0D] = NULL,
	[0x0E] = NULL,
	[0x0F] = NULL,
	[0x10] = branch,
	[0x11] = NULL,
	[0x12] = NULL,
	[0x13] = NULL,
	[0x14] = NULL,
	[0x15] = NULL,
	[0x16] = NULL,
	[0x17] = NULL,
	[0x18] = proc_flag_set_clear,
	[0x19] = NULL,
	[0x1A] = NULL,
	[0x1B] = NULL,
	[0x1C] = NULL,
	[0x1D] = NULL,
	[0x1E] = NULL,
	[0x1F] = NULL,
	[0x20] = jump_to_subroutine,
	[0x21] = NULL,
	[0x22] = NULL,
	[0x23] = NULL,
	[0x24] = NULL,
	[0x25] = NULL,
	[0x26] = NULL,
	[0x27] = NULL,
	[0x28] = NULL,
	[0x29] = NULL,
	[0x2A] = NULL,
	[0x2B] = NULL,
	[0x2C] = NULL,
	[0x2D] = NULL,
	[0x2E] = NULL,
	[0x2F] = NULL,
	[0x30] = branch,
	[0x31] = NULL,
	[0x32] = NULL,
	[0x33] = NULL,
	[0x34] = NULL,
	[0x35] = NULL,
	[0x36] = NULL,
	[0x37] = NULL,
	[0x38] = proc_flag_set_clear,
	[0x39] = NULL,
	[0x3A] = NULL,
	[0x3B] = NULL,
	[0x3C] = NULL,
	[0x3D] = NULL,
	[0x3E] = NULL,
	[0x3F] = NULL,
	[0x40] = NULL,
	[0x41] = NULL,
	[0x42] = NULL,
	[0x43] = NULL,
	[0x44] = NULL,
	[0x45] = NULL,
	[0x46] = NULL,
	[0x47] = NULL,
	[0x48] = NULL,
	[0x49] = NULL,
	[0x4A] = NULL,
	[0x4B] = NULL,
	[0x4C] = jmp_absolute,
	[0x4D] = NULL,
	[0x4E] = NULL,
	[0x4F] = NULL,
	[0x50] = branch,
	[0x51] = NULL,
	[0x52] = NULL,
	[0x53] = NULL,
	[0x54] = NULL,
	[0x55] = NULL,
	[0x56] = NULL,
	[0x57] = NULL,
	[0x58] = proc_flag_set_clear,
	[0x59] = NULL,
	[0x5A] = NULL,
	[0x5B] = NULL,
	[0x5C] = NULL,
	[0x5D] = NULL,
	[0x5E] = NULL,
	[0x5F] = NULL,
	[0x60] = NULL,
	[0x61] = NULL,
	[0x62] = NULL,
	[0x63] = NULL,
	[0x64] = NULL,
	[0x65] = NULL,
	[0x66] = NULL,
	[0x67] = NULL,
	[0x68] = NULL,
	[0x69] = NULL,
	[0x6A] = NULL,
	[0x6B] = NULL,
	[0x6C] = NULL,
	[0x6D] = NULL,
	[0x6E] = NULL,
	[0x6F] = NULL,
	[0x70] = branch,
	[0x71] = NULL,
	[0x72] = NULL,
	[0x73] = NULL,
	[0x74] = NULL,
	[0x75] = NULL,
	[0x76] = NULL,
	[0x77] = NULL,
	[0x78] = proc_flag_set_clear,
	[0x79] = NULL,
	[0x7A] = NULL,
	[0x7B] = NULL,
	[0x7C] = NULL,
	[0x7D] = NULL,
	[0x7E] = NULL,
	[0x7F] = NULL,
	[0x80] = NULL,
	[0x81] = store,
	[0x82] = NULL,
	[0x83] = NULL,
	[0x84] = store,
	[0x85] = store,
	[0x86] = store,
	[0x87] = NULL,
	[0x88] = transfer_register,
	[0x89] = NULL,
	[0x8A] = transfer_register,
	[0x8B] = NULL,
	[0x8C] = store,
	[0x8D] = store,
	[0x8E] = store,
	[0x8F] = NULL,
	[0x90] = branch,
	[0x91] = store,
	[0x92] = NULL,
	[0x93] = NULL,
	[0x94] = store,
	[0x95] = store,
	[0x96] = store,
	[0x97] = NULL,
	[0x98] = transfer_register,
	[0x99] = store,
	[0x9A] = transfer_register,
	[0x9B] = NULL,
	[0x9C] = NULL,
	[0x9D] = store,
	[0x9E] = NULL,
	[0x9F] = NULL,
	[0xA0] = ld_immediate,
	[0xA1] = NULL,
	[0xA2] = ld_immediate,
	[0xA3] = NULL,
	[0xA4] = NULL,
	[0xA5] = NULL,
	[0xA6] = NULL,
	[0xA7] = NULL,
	[0xA8] = transfer_register,
	[0xA9] = ld_immediate,
	[0xAA] = transfer_register,
	[0xAB] = NULL,
	[0xAC] = NULL,
	[0xAD] = NULL,
	[0xAE] = NULL,
	[0xAF] = NULL,
	[0xB0] = branch,
	[0xB1] = NULL,
	[0xB2] = NULL,
	[0xB3] = NULL,
	[0xB4] = NULL,
	[0xB5] = NULL,
	[0xB6] = NULL,
	[0xB7] = NULL,
	[0xB8] = proc_flag_set_clear,
	[0xB9] = NULL,
	[0xBA] = transfer_register,
	[0xBB] = NULL,
	[0xBC] = NULL,
	[0xBD] = NULL,
	[0xBE] = NULL,
	[0xBF] = NULL,
	[0xC0] = NULL,
	[0xC1] = NULL,
	[0xC2] = NULL,
	[0xC3] = NULL,
	[0xC4] = NULL,
	[0xC5] = NULL,
	[0xC6] = NULL,
	[0xC7] = NULL,
	[0xC8] = transfer_register,
	[0xC9] = NULL,
	[0xCA] = transfer_register,
	[0xCB] = NULL,
	[0xCC] = NULL,
	[0xCD] = NULL,
	[0xCE] = NULL,
	[0xCF] = NULL,
	[0xD0] = branch,
    [0xD1] = NULL,
    [0xD2] = NULL,
    [0xD3] = NULL,
    [0xD4] = NULL,
    [0xD5] = NULL,
    [0xD6] = NULL,
    [0xD7] = NULL,
    [0xD8] = proc_flag_set_clear,
    [0xD9] = NULL,
    [0xDA] = NULL,
    [0xDB] = NULL,
    [0xDC] = NULL,
    [0xDD] = NULL,
    [0xDE] = NULL,
    [0xDF] = NULL,
	[0xE0] = NULL,
	[0xE1] = NULL,
	[0xE2] = NULL,
	[0xE3] = NULL,
	[0xE4] = NULL,
	[0xE5] = NULL,
	[0xE6] = NULL,
	[0xE7] = NULL,
	[0xE8] = transfer_register,
	[0xE9] = NULL,
	[0xEA] = NULL,
	[0xEB] = NULL,
	[0xEC] = NULL,
	[0xED] = NULL,
	[0xEE] = NULL,
	[0xEF] = NULL,
	[0xF0] = branch,
	[0xF1] = NULL,
	[0xF2] = NULL,
	[0xF3] = NULL,
	[0xF4] = NULL,
	[0xF5] = NULL,
	[0xF6] = NULL,
	[0xF7] = NULL,
	[0xF8] = proc_flag_set_clear,
	[0xF9] = NULL,
	[0xFA] = NULL,
	[0xFB] = NULL,
	[0xFC] = NULL,
	[0xFD] = NULL,
	[0xFE] = NULL,
	[0xFF] = NULL,
};

opcode_t get_opcode_jump_table(uint8_t opcode)
{
    return opcode_jump_table[opcode];
}