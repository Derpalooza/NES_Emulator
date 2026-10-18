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
			cpu->PC.addr++;
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
			cpu->PC.addr++;
			cpu->effective_address.addr = cpu->PC.addr;
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
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		case 1:
			// Read lower byte of effective address
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->effective_address.bytes.low_byte = operand_byte;
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		case 2:
			// Read upper byte of effective address
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->effective_address.bytes.high_byte = operand_byte;
			cpu->PC.addr++;
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
			cpu->PC.addr++;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC.addr, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->effective_address.addr = zp_addr;
			cpu->PC.addr++;
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
			cpu->PC.addr++;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status = memory_read_byte(memory, cpu->PC.addr, &zp_addr);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			cpu->operand = zp_addr;
			cpu->PC.addr++;
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
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC.addr++;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status_t status_mem = memory_read_byte(memory, cpu->PC.addr, &zp_addr);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			cpu->operand = zp_addr;
			cpu->PC.addr++;
			break;
		}
		case 2:
		{
			// Initiate dummy read on initial zero page address and discard results
			uint8_t operand_byte = 0;
			status_t status_mem = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			// Add X to operand_addr. In indirect X mode, we look in zero page, so
			// the address will just wrap around if an overflow happens. This means no page fault will
			// happen
			cpu->operand = (cpu->operand + cpu->X) & 0xFF;
			break;
		}
		case 3:
		{
			// Get low bit of effective address
			uint8_t operand_byte = 0;
			status_t status_mem = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			cpu->effective_address.bytes.low_byte = operand_byte;
			cpu->operand = (cpu->operand + 1) & 0xFF;
			break;
		}
		case 4:
		{
			// Get high bit of effective address
			uint8_t operand_byte = 0;
			status_t status_mem = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			cpu->effective_address.bytes.high_byte = operand_byte;
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
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
			cpu->PC.addr++;
			break;
		case 1:
		{
			// Obtain zero page address
			uint8_t zp_addr = 0;
			status_t status_mem = memory_read_byte(memory, cpu->PC.addr, &zp_addr);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			cpu->operand = zp_addr;
			cpu->PC.addr++;
			break;
		}
		case 2:
		{
			// Fetch low byte of effective address
			uint8_t operand_byte = 0;
			status_t status_mem = memory_read_byte(memory, cpu->operand, &operand_byte);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}
			
			cpu->effective_address.bytes.low_byte = operand_byte;

			// The effective address is stored in the zero page, so make sure it wraps around
			cpu->operand = (cpu->operand + 1) & 0xFF;
			break;
		}
		case 3:
		{
			// Fetch high byte of effective address
			uint8_t high_byte = 0;
			status_t status_mem = memory_read_byte(memory, cpu->operand, &high_byte);
			if (status_mem != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status_mem);
				return status_mem;
			}

			// Save high byte
			cpu->effective_address.bytes.high_byte = high_byte;

			// Construct effective address by adding Y to entire saved address
			uint16_t expected_address = cpu->effective_address.addr + cpu->Y;
			
			// Add Y to lower byte
			cpu->effective_address.bytes.low_byte += cpu->Y;

			// The expected effective address should be equal to the actual effective address. If this isn't the case, a page
			// fault has occurred
			if (cpu->effective_address.addr != expected_address)
			{
				// A page fault has occured. Save the calculated effective addres for use in the next cycle
				cpu->operand = expected_address;
			}
			else
			{
				cpu->operand = cpu->effective_address.addr;
				return STATUS_SUCCESS;
			}
			break;
		}
		default:
		{
			// Read from effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->effective_address.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// A page fault occurred in the last cycle. So reapply the saved high byte
			cpu->effective_address.addr = cpu->operand;
			break;
		}
	}
	
    return status;
}

status_t get_operand_absolute_indexed(cpu_t* cpu, Memory_t* memory, cpu_addr_mode_t addr_mode)
{
	status_t status = STATUS_INPROGRESS;
	uint8_t operand_byte = 0;
	
	if ((addr_mode != CPU_ADDR_MODE_ABSOLUTE_Y) && (addr_mode != CPU_ADDR_MODE_ABSOLUTE_X))
	{
		printf("Invalid addressing mode for this function (rc = %d)\n", addr_mode);
		return STATUS_EINVALIDPARAMS;
	}
	
	uint8_t* reg = (addr_mode == CPU_ADDR_MODE_ABSOLUTE_X) ? &cpu->X : &cpu->Y;
	
	switch (cpu->current_opcode_cycles)
	{
		case 0:
		{
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		}
		case 1:
		{
			// Read lower byte of effective address
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->effective_address.bytes.low_byte = operand_byte;
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		}
		case 2:
		{
			// Read upper byte of effective address
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->PC.addr++;
			
			cpu->effective_address.bytes.high_byte = operand_byte;		
			
			// Calculate the expected effective address
			uint16_t expected_address = cpu->effective_address.addr + *reg; 
			
			// Add register to low byte 
			cpu->effective_address.addr += *reg;
			
			// The expected effective address should be equal to the actual effective address. If this isn't the case, a page
			// fault has occurred
			if (expected_address == cpu->effective_address.addr)
			{
				// A page fault has occured. Save the calculated effective addres for use in the next cycle
				cpu->operand = expected_address;
			}
			else
			{
				cpu->operand = cpu->effective_address.addr;
				return STATUS_SUCCESS;
			}
			break;
		}
		case 3:
		{
			// Execute dummy read from effective address
			uint8_t operand_byte = 0;
			status = memory_read_byte(memory, cpu->effective_address.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			
			// A page fault occurred in the last cycle. So reapply the saved high byte
			cpu->effective_address.addr = cpu->operand;
			break;
		}
		default:
			printf("Exceeded number of cycles (%d cycles) for this addressing mode\n", cpu->current_opcode_cycles);
			status = STATUS_EINVALIDPARAMS;
			break;
	}

    return status;
}

status_t get_operand(cpu_t* cpu, Memory_t* memory, cpu_addr_mode_t addr_mode)
{
	status_t status = STATUS_SUCCESS;

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
			status = get_operand_absolute_indexed(cpu, memory, CPU_ADDR_MODE_ABSOLUTE_Y);
			break;
		case CPU_ADDR_MODE_ABSOLUTE_X:
			status = get_operand_absolute_indexed(cpu, memory, CPU_ADDR_MODE_ABSOLUTE_X);
			break;
		default:
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
					cpu_print_instruction(cpu->effective_address.addr, "JMP", CPU_ADDR_MODE_ABSOLUTE);
                    cpu->PC.addr = cpu->effective_address.addr;
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
	
	cpu_register_t target_register;
	status = cpu_get_opcode_register(cpu->current_opcode, &target_register);
	if (status != STATUS_SUCCESS)
	{
		printf("cpu_get_opcode_register failed (status = %d)", status);
		return status;
	}
	
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

	cpu_addr_mode_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
            status = get_operand(cpu, memory, addr_mode);
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
            status = get_operand(cpu, memory, addr_mode);
            switch (status)
            {
                case STATUS_SUCCESS:
					if ((addr_mode == CPU_ADDR_MODE_INDIRECT_Y) && (cpu->current_opcode_cycles == 3))
					{
						// For indirect Y mode, store takes 6 cycles instead of 5+. If no page fault occured (4 cycles passed), we need
						// to stall for another cycle
						cpu->state = CPU_STATE_DECODE;
					}
					else if ((addr_mode == CPU_ADDR_MODE_ABSOLUTE_Y || addr_mode == CPU_ADDR_MODE_ABSOLUTE_X) && (cpu->current_opcode_cycles == 2))
					{
						// In absolute indexed mode, store takes 5 cycles instead of 4+. If no page fault occured (3 cycles passed), we need
						// to stall for another cycle
						cpu->state = CPU_STATE_DECODE;
					}
					else
					{
                    	cpu->state = CPU_STATE_EXECUTE;
					}
                    break;
                case STATUS_INPROGRESS:
                    status = STATUS_SUCCESS;
                    break;
                default:
                    printf("get_operand failed with error %d\n", status);
            }
            break;
        case CPU_STATE_EXECUTE:
            memory_write_byte(memory, cpu->effective_address.addr, *reg);

			// Print opcode information
			char instr_name[8] = {};
			sprintf(instr_name, "ST%s", (target_register == CPU_REGISTER_Y) ? "Y" : (target_register == CPU_REGISTER_A) ? "A" : "X");
			cpu_print_instruction(cpu->effective_address.addr, instr_name, addr_mode);

            cpu->state = CPU_STATE_FETCH;
            break;
        default:
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
    }

    return status;
}

status_t load(cpu_t* cpu, Memory_t* memory)
{
    status_t status = STATUS_SUCCESS;

	cpu_register_t target_register;
	status = cpu_get_opcode_register(cpu->current_opcode, &target_register);
	if (status != STATUS_SUCCESS)
	{
		printf("cpu_get_opcode_register failed (status = %d)", status);
		return status;
	}

	cpu_addr_mode_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);

	if ((addr_mode == CPU_ADDR_MODE_INDIRECT_X) && (target_register != CPU_REGISTER_A))
	{
		// The Load X and Y immediate instructions hold "indirect X" as their addressing mode. That
		// must be corrected here
		addr_mode = CPU_ADDR_MODE_IMMEDIATE;
	}

    switch (cpu->state)
    {
        case CPU_STATE_FETCH:
		{
			status_t addr_result = get_operand(cpu, memory, addr_mode);

			switch (addr_result)
			{
				case STATUS_INPROGRESS:
					cpu->state = CPU_STATE_DECODE;
					break;
				case STATUS_SUCCESS:
					cpu->state = CPU_STATE_EXECUTE;
					break;
				default:
					printf("get_operand failed with %d\n", addr_result);
					status = addr_result;
			}
            break;
		}
		case CPU_STATE_DECODE:
		{
			status_t addr_result = get_operand(cpu, memory, addr_mode);
			switch (addr_result)
			{
				case STATUS_INPROGRESS:
					break;
				case STATUS_SUCCESS:
					cpu->state = CPU_STATE_EXECUTE;
					break;
				default:
					printf("get_operand failed with %d\n", addr_result);
					status = addr_result;
			}
		}
        case CPU_STATE_EXECUTE:
		{
			uint8_t loaded_byte = 0;
			status = memory_read_byte(memory, cpu->effective_address.addr, &loaded_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d", status);
				return status;
			}

			status = cpu_set_register(cpu, target_register, loaded_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("cpu_set_register failed with error %d", status);
				return status;
			}
			
			// Print opcode information
			char instr_name[8] = {};
			sprintf(instr_name, "LD%s", (target_register == CPU_REGISTER_Y) ? "Y" : (target_register == CPU_REGISTER_A) ? "A" : "X");
			cpu_print_instruction(loaded_byte, instr_name, addr_mode);

            cpu->state = CPU_STATE_FETCH;
			cpu->PC.addr++;
            break;
		}
        default:
		{
            printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
		}
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
			cpu->PC.addr++;
            break;
        case CPU_STATE_EXECUTE:
			switch (cpu->current_opcode)
			{
				case 0xA8:
					// Transfer Accumulator to Y
					cpu_set_register(cpu, CPU_REGISTER_Y, cpu->A);
					cpu_print_instruction(cpu->operand, "TAY", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xAA:
					// Transfer Accumulator to X
					cpu_set_register(cpu, CPU_REGISTER_X, cpu->A);
					cpu_print_instruction(cpu->operand, "TAX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xBA:
					// Transfer Stack pointer to X
					cpu_set_register(cpu, CPU_REGISTER_X, cpu->SP);
					cpu_print_instruction(cpu->operand, "TSX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x98:
					// Transfer Y to Accumulator
					cpu_set_register(cpu, CPU_REGISTER_A, cpu->Y);
					cpu_print_instruction(cpu->operand, "TYA", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x8A:
					// Transfer X to Accumulator
					cpu_set_register(cpu, CPU_REGISTER_A, cpu->X);
					cpu_print_instruction(cpu->operand, "TXA", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x9A:
					// Transfer X to Stack pointer
					cpu_set_register(cpu, CPU_REGISTER_SP, cpu->X);
					cpu_print_instruction(cpu->operand, "TXS", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xE8:
					// Increment X
					cpu_set_register(cpu, CPU_REGISTER_X, cpu->X + 1);
					cpu_print_instruction(cpu->operand, "INX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xC8:
					// Increment Y
					cpu_set_register(cpu, CPU_REGISTER_Y, cpu->Y + 1);
					cpu_print_instruction(cpu->operand, "INY", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0xCA:
					// Decrement X
					cpu_set_register(cpu, CPU_REGISTER_X, cpu->X - 1);
					cpu_print_instruction(cpu->operand, "DEX", CPU_ADDR_MODE_IMPLIED);
					break;
				case 0x88:
					// Decrement Y
					cpu_set_register(cpu, CPU_REGISTER_Y, cpu->Y - 1);
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
			cpu->PC.addr++;
			status = STATUS_SUCCESS;
			break;
		}
		case 1:
		{
			// Fetch low address byte of subroutine address
			uint8_t low_addr_byte = 0;
			status = memory_read_byte(memory, cpu->PC.addr, &low_addr_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d", status);
				return status;
			}
			cpu->effective_address.bytes.low_byte = low_addr_byte;
			cpu->PC.addr++;
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
			status = memory_write_byte(memory, cpu->SP, cpu->PC.bytes.high_byte);
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
			status = memory_write_byte(memory, cpu->SP, cpu->PC.bytes.low_byte);
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
			status = memory_read_byte(memory, cpu->PC.addr, &high_addr_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_write_byte failed with error %d", status);
				return status;
			}
			cpu->effective_address.bytes.high_byte = high_addr_byte;
			cpu->PC.addr = cpu->effective_address.addr;
			cpu->state = CPU_STATE_FETCH;
			cpu_print_instruction(cpu->effective_address.addr, "JSR", CPU_ADDR_MODE_ABSOLUTE);
			break;
		}
	}

    return status;
}

status_t return_from_subroutine(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	// The standard state machine seen with the other instructions isn't used
	// for RTS. Because it doesn't collect its operands in a straightforward addressing
	// mode but rather spends cycles updating the stack (MCS6500 MICROCOMPUTER FAMILY PROGRAMMING MANUAL in DOCS folder)
	switch (cpu->current_opcode_cycles)
	{
		case 0:
		{
			cpu->state = CPU_STATE_DECODE;
			cpu->PC.addr++;
			status = STATUS_SUCCESS;
			break;
		}
		case 1:
		{
			// Fetch low address byte of subroutine address
			uint8_t low_addr_byte = 0;
			status = memory_read_byte(memory, cpu->PC.addr, &low_addr_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d", status);
				return status;
			}
			break;
		}
		case 2:
		{
			cpu->SP++;
			break;
		}
		case 3:
		{
			// Pull low byte of PC from stack
			uint8_t PC_low_byte = 0;
			status = memory_read_byte(memory, cpu->SP, &PC_low_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d", status);
				return status;
			}
			
			cpu->PC.bytes.low_byte = PC_low_byte;
			cpu->SP++;
			break;
		}
		case 4:
		{
			// Pull high byte of PC from stack
			uint8_t PC_high_byte = 0;
			status = memory_read_byte(memory, cpu->SP, &PC_high_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d", status);
				return status;
			}
			
			cpu->PC.bytes.high_byte = PC_high_byte;
			cpu->SP++;
			break;
		}
		case 5:
		{
			cpu->PC.addr++;
			cpu->state = CPU_STATE_FETCH;
			cpu_print_instruction(cpu->PC.addr, "RTS", CPU_ADDR_MODE_IMPLIED);
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
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		case 1:
			// Read offset byte stored in operand
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with error %d\n", status);
				return status;
			}
			cpu->operand = operand_byte;
			cpu->PC.addr++;
			status = STATUS_INPROGRESS;
			break;
		case 2:
			// Perform dummy read of PC
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);

			// Calculate new PC (the branch offset must be signed for relative addressing, as it is a +/- offset)
			int8_t offset = cpu->operand & 0xFF;

			// Calculate the true branch address
			uint16_t new_PC = cpu->PC.addr + offset;

			// Add offset to PC lower byte
			cpu->PC.bytes.low_byte += offset;

			// Compare the true branch address to the updated PC. If the two are not equal, a page fault occured
			if (new_PC == cpu->PC.addr)
			{
				// If the upper bytes are equal, then no page fault happened. We can end the relative addressing operation
				status = STATUS_SUCCESS;
			}
			else
			{
				// If a page fault occurred, save the full new address for the next cycle
				cpu->operand = new_PC;
			}
			break;
		case 3:
			// Perform dummy read of PC
			status = memory_read_byte(memory, cpu->PC.addr, &operand_byte);

			// Update PC with full new address
			cpu->PC.addr = cpu->operand;
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

	cpu_addr_mode_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
		{	
			cpu->state = CPU_STATE_DECODE;
			status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			}
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
			break;
		}
		case CPU_STATE_DECODE:
		{
            status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_SUCCESS)
			{
				cpu->state = CPU_STATE_READ;
			}
			else if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			} 
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
            break;
		}
		case CPU_STATE_READ:
		{
			// Read from the effective address
			uint8_t value = 0;
			status_t operand_status = memory_read_byte(memory, cpu->effective_address.addr, &value);
			if (operand_status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with status %d\n", operand_status);
				status = operand_status;
				break;
			}

			cpu->operand = value;
			cpu->state = CPU_STATE_EXECUTE;
			break;
		}
        case CPU_STATE_EXECUTE:
		{
			// This is a read-modify-write instruction. Must perform a dummy write of the operand
			// to the effective address before modifying the operand
			status_t operand_status = memory_write_byte(memory, cpu->effective_address.addr, cpu->operand);
			if (operand_status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with status %d\n", operand_status);
				status = operand_status;
				break;
			}

			cpu->operand = cpu_add_data(cpu, cpu->operand, 1);
			cpu->state = CPU_STATE_WRITE;
			break;
		}
		case CPU_STATE_WRITE:
		{
			// Write the newly incremented value to memory
			status_t operand_status = memory_write_byte(memory, cpu->effective_address.addr, cpu->operand);
			if (operand_status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with status %d\n", status);
				status = operand_status;
				break;
			}

			// Print instruction			
			cpu_print_instruction(cpu->operand, "INC", addr_mode);
			cpu->state = CPU_STATE_FETCH;
			break;
		}
		default:
		{
			printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
			break;
		}
	}
    
    return status;
}

status_t test_bits(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	cpu_addr_mode_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
		{	
			cpu->state = CPU_STATE_DECODE;
			status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			}
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
			break;
		}
		case CPU_STATE_DECODE:
		{
            status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_SUCCESS)
			{
				cpu->state = CPU_STATE_EXECUTE;
			}
			else if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			} 
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
            break;
		}
        case CPU_STATE_EXECUTE:
		{
			uint8_t memory_byte = 0;
			status = memory_read_byte(memory, cpu->effective_address.addr, &memory_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with status %d\n", status);
				break;
			}
			
			cpu->operand = memory_byte & cpu->A;
			
			if (cpu->operand & STATUS_NEGATIVE_MASK)
			{
				cpu->P |= STATUS_NEGATIVE_MASK;
			}
		
			if (cpu->operand & STATUS_OVERFLOW_MASK)
			{
				cpu->P |= STATUS_OVERFLOW_MASK;
			}
		
			if (cpu->operand == 0)
			{
				cpu->P |= STATUS_ZERO_FLAG_MASK;
			}
			
			cpu_print_instruction(cpu->operand, "BIT", addr_mode);
			cpu->state = CPU_STATE_FETCH;
			break;
		}
		default:
		{
			printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
			break;
		}
	}
    
    return status;
}

status_t or_accumulator(cpu_t* cpu, Memory_t* memory)
{
	status_t status = STATUS_SUCCESS;

	cpu_addr_mode_t addr_mode = cpu_get_opcode_addressing_mode(cpu->current_opcode);

	switch (cpu->state)
    {
        case CPU_STATE_FETCH:
		{	
			cpu->state = CPU_STATE_DECODE;
			status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			}
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
			break;
		}
		case CPU_STATE_DECODE:
		{
            status_t operand_status = get_operand(cpu, memory, addr_mode);
			if (operand_status == STATUS_SUCCESS)
			{
				cpu->state = CPU_STATE_EXECUTE;
			}
			else if (operand_status == STATUS_INPROGRESS)
			{
				cpu->state = CPU_STATE_DECODE;
			} 
			else
			{
				printf("Invalid get_operand result (rc = %d)\n", operand_status);
				status = operand_status;
			}
            break;
		}
        case CPU_STATE_EXECUTE:
		{
			uint8_t memory_byte = 0;
			status = memory_read_byte(memory, cpu->effective_address.addr, &memory_byte);
			if (status != STATUS_SUCCESS)
			{
				printf("memory_read_byte failed with status %d\n", status);
				break;
			}
			
			uint8_t accumulator = cpu->A | memory_byte;
			status = cpu_set_register(cpu, CPU_REGISTER_A, accumulator);
			if (status != STATUS_SUCCESS)
			{
				printf("cpu_set_register failed with status %d\n", status);
				break;
			}
			
			
			cpu_print_instruction(accumulator, "ORA", addr_mode);
			cpu->state = CPU_STATE_FETCH;
			break;
		}
		default:
		{
			printf("CPU state %d is invalid for opcode 0x%02x\n", cpu->state, cpu->current_opcode);
            status = STATUS_EINVALIDPARAMS;
			break;
		}
	}
    
    return status;
}

static opcode_t opcode_jump_table[] = {
	[0x00] = or_accumulator,
	[0x01] = NULL,
	[0x02] = NULL,
	[0x03] = NULL,
	[0x04] = NULL,
	[0x05] = or_accumulator,
	[0x06] = NULL,
	[0x07] = NULL,
	[0x08] = NULL,
	[0x09] = or_accumulator,
	[0x0A] = NULL,
	[0x0B] = NULL,
	[0x0C] = NULL,
	[0x0D] = or_accumulator,
	[0x0E] = NULL,
	[0x0F] = NULL,
	[0x10] = branch,
	[0x11] = or_accumulator,
	[0x12] = NULL,
	[0x13] = NULL,
	[0x14] = NULL,
	[0x15] = or_accumulator,
	[0x16] = NULL,
	[0x17] = NULL,
	[0x18] = proc_flag_set_clear,
	[0x19] = or_accumulator,
	[0x1A] = NULL,
	[0x1B] = NULL,
	[0x1C] = NULL,
	[0x1D] = or_accumulator,
	[0x1E] = NULL,
	[0x1F] = NULL,
	[0x20] = jump_to_subroutine,
	[0x21] = NULL,
	[0x22] = NULL,
	[0x23] = NULL,
	[0x24] = test_bits,
	[0x25] = NULL,
	[0x26] = NULL,
	[0x27] = NULL,
	[0x28] = NULL,
	[0x29] = NULL,
	[0x2A] = NULL,
	[0x2B] = NULL,
	[0x2C] = test_bits,
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
	[0x60] = return_from_subroutine,
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
	[0xA0] = load,
	[0xA1] = NULL,
	[0xA2] = load,
	[0xA3] = NULL,
	[0xA4] = NULL,
	[0xA5] = NULL,
	[0xA6] = NULL,
	[0xA7] = NULL,
	[0xA8] = transfer_register,
	[0xA9] = load,
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
	[0xE6] = increment,
	[0xE7] = NULL,
	[0xE8] = transfer_register,
	[0xE9] = NULL,
	[0xEA] = NULL,
	[0xEB] = NULL,
	[0xEC] = NULL,
	[0xED] = NULL,
	[0xEE] = increment,
	[0xEF] = NULL,
	[0xF0] = branch,
	[0xF1] = NULL,
	[0xF2] = NULL,
	[0xF3] = NULL,
	[0xF4] = NULL,
	[0xF5] = NULL,
	[0xF6] = increment,
	[0xF7] = NULL,
	[0xF8] = proc_flag_set_clear,
	[0xF9] = NULL,
	[0xFA] = NULL,
	[0xFB] = NULL,
	[0xFC] = NULL,
	[0xFD] = NULL,
	[0xFE] = increment,
	[0xFF] = NULL,
};

opcode_t get_opcode_jump_table(uint8_t opcode)
{
    return opcode_jump_table[opcode];
}