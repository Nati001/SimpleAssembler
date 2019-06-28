#include "second_transition.h"
#include "asm_parsing.h"
#include "asm_data.h"
#include <stdio.h>
#include <stdlib.h>

/*Add a binary code contains information of 2 registers operands.*/
void add_registers_word(char* op1, char* op2, int index)
{
	char* word = calloc(WORD_SIZE, sizeof(char));

	char* to_fill = word;
	/*Fill the second register to the 6 left bits.*/
	int_to_binary(atoi(op2), to_fill, 6); 
	to_fill += 6;
	/*Fill the first register to the 6 next bits*/
	int_to_binary(atoi(op1), to_fill, 6);
	to_fill += 6;
	/*Registers are always absolute*/
	int_to_binary(C_ABSOLUTE, to_fill, 2);

	add_code(word, index);
}

/*Add a word of a single operand.*/
bool add_word(addressing addr, char* op, int index, bool first, int line_num)
{
	char* word = calloc(WORD_SIZE, sizeof(char));
	char* to_fill = word;
	coding_type c_type = C_ABSOLUTE;
	int label = -1;

	if (addr == DIRECT || addr == JAMP_WITH_PRAMTER)
	{
		/*Then there's a label and we need to validate if exists.*/
		label = validate_and_find_label(op, line_num);
		if (label == -1)
			return false;
		
		c_type = C_RELOCATE;
		if (is_extern_symbol(op))
		{
			/*Save the IC where the extern symbol is used for the ext file.*/
			ext_sym_use(op, index);
			c_type = C_EXTERN;
		}
	}
	
	switch (addr)
	{
	case IMMEDIATE:
		/*In case of immediate just put the int in 12 bits.*/
		int_to_binary(atoi(op), to_fill, 12);
		to_fill += 12;
		break;
	case DIRECT:
	case JAMP_WITH_PRAMTER:
		/*In case of direct put the label location in 12 bits.*/
		int_to_binary(label, to_fill, 12);
		to_fill += 12;
		break;
	case REGISTER:
		/*order the register in the right 6 bits according the number of the operand.*/
		if (first)
		{
			int_to_binary(atoi(op), to_fill, 6);
			to_fill += 6;
			int_to_binary(0, to_fill, 6);
		}
		else
		{
			int_to_binary(0, to_fill, 6);
			to_fill += 6;
			int_to_binary(atoi(op), to_fill, 6);
		}
		to_fill += 6;
		break;
	default:
		return false;
	}
	/*Add A,E,R bits*/
	int_to_binary(c_type, to_fill, 2);

	add_code(word, index);
	return true;
}

bool perform_second_transion()
{
	instruction* curr_inst = get_first_instr();
	bool ok = true;
	bool one_op;

	/*Iterate through the instruction*/
	while (curr_inst)
	{
		/*If there's 2 registers parameters*/
		if (curr_inst->op_d != NULL && curr_inst->op_d_addr == REGISTER && curr_inst->op_s_addr == REGISTER)
			add_registers_word(curr_inst->op_d, curr_inst->op_s, curr_inst->ic+1);
		else if (curr_inst->op_d != NULL) /*if there's a parameter at all*/
		{
			/*do we have 2 operands or 1?*/
			one_op = !curr_inst->op_s;
			/*Add destination operand word*/
			ok &= add_word(curr_inst->op_d_addr, curr_inst->op_d, curr_inst->ic + (one_op ? 1: 2), one_op, curr_inst->line_num);
			if (!one_op)
				/*Add source operand word*/
				ok &= add_word(curr_inst->op_s_addr, curr_inst->op_s, curr_inst->ic + 1, true, curr_inst->line_num);
			else if (curr_inst->op_d_addr == JAMP_WITH_PRAMTER) /*If we are in Jump with Addressing method we should add the 2 params.*/
			{
				if (curr_inst->jwp_p1_addr == REGISTER && curr_inst->jwp_p2_addr == REGISTER)
					add_registers_word(curr_inst->jwp_p2, curr_inst->jwp_p1, curr_inst->ic + 2);
				else
				{
					ok &= add_word(curr_inst->jwp_p1_addr, curr_inst->jwp_p1, curr_inst->ic + 2, true, curr_inst->line_num);
					ok &= add_word(curr_inst->jwp_p2_addr, curr_inst->jwp_p2, curr_inst->ic + 3, false, curr_inst->line_num);
				}
			}
		}
		curr_inst = get_next_instr();
	}

	return ok;
}