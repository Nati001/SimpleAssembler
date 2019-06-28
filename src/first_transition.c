#include "utils.h"
#include "asm_data.h"
#include "asm_parsing.h"
#include "first_transition.h"
#include "operators.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Parse a directive sentence, add symbols and data accordingly and return false if it's invalid*/
bool parse_directive(char* directive, char* params, char* label, int* DC, int line_num)
{
	int* data = NULL;
	int data_size;
	directive_type type;
		
	params = remove_whitespace(params);
	type = get_directive_type(directive);
	/*parse the directive, extract tha data if needed and return false if invalid.*/
	if (!is_valid_directive_params(type, params, line_num, &data, &data_size))
		return false;

	switch (type)
	{
	case D_DATA:
	case D_STRING:
		if (label[0] != '\0') /*if there's a label*/
			add_data_symbol(label, *DC);
		add_data(type, data, data_size);
		*DC += data_size;/* increase the data counter by the size of data inserted*/
		break;
	
	case D_ENTRY:
		/*save the label as entry.*/
		save_entry_label(params, line_num);
		break;

	case D_EXTERN:
		add_extern_symbol(params);
		break;
	default:
		return false;
	}
	return true;
}

/*Construct the first word of the binary code of instruction.*/
char* construct_first_word(instruction* inst)
{
	char* word = calloc(WORD_SIZE, sizeof(char));

	char* to_fill = word;

	/*add jump with addresing first parameter to the left 2 bytes*/
	int_to_binary(inst->jwp_p1_addr, to_fill, 2);
	to_fill += 2;
	/*add jump with addresing second parameter to the next 2 bytes*/
	int_to_binary(inst->jwp_p2_addr, to_fill, 2); 
	to_fill += 2;
	/*add the op code to the next 4 bytes*/
	int_to_binary(inst->op_code, to_fill, 4); 
	to_fill += 4;
	/*add the source operand addresing to the next 2 bytes*/
	int_to_binary((inst->op_s_addr == NONE) ? 0 : inst->op_s_addr, to_fill, 2);
	to_fill += 2;
	/*add the destination operand addresing to the next 2 bytes*/
	int_to_binary(inst->op_d_addr, to_fill, 2);
	to_fill += 2;
	/*add tA to the right 2 bytes - since the first word of an instruction is always absolute*/
	int_to_binary(C_ABSOLUTE, to_fill, 2);

	return word;
}

/*Calculates the number of words the instruction is consists from*/
int calc_num_of_words(instruction* inst)
{
	if (inst->op_d_addr == JAMP_WITH_PRAMTER)
		return (inst->jwp_p1_addr == REGISTER && inst->jwp_p2_addr == REGISTER) ? 3 : 4;
	if (inst->op_s == NULL) /* No source operand*/
		return (inst->op_d == NULL) ? 1: 2;
	if (inst->op_d_addr == REGISTER && inst->op_d_addr == REGISTER)
		return 2;
	return 3;
}

/*Parse an insruction sentence, add symbols and binary code accordingly and return false if it's invalid*/
bool parse_instruction(char* op_name, char* params, int line_num, char* label, int* IC)
{
	instruction* inst = calloc(1, sizeof(instruction));
	asm_operator* op = find_valid_op(op_name, line_num);
	
	/*Parse and validate instruction. returns the parsed data in inst.*/
	if (!op || !is_valid_instruction(op, params, inst, line_num))
	{
		free(inst);
		return false;
	}

	/*add the current ic since the second transition only iterates through the instruction objects.*/
	inst->ic = *IC;
	inst->line_num = line_num;
	
	/*add the first word of the instruction*/
	add_code(construct_first_word(inst), *IC);
	*IC += calc_num_of_words(inst);

	/*add the instruction objects to the internal data structure*/
	add_instruction(inst);
	return true;
}

/*Parse a single assenbly line and return false if invalid*/
bool parse_line(char* line, int line_num, int* IC, int* DC)
{
	char* r_line;
	char word[81];
	char label[32] = "";
	
	if (!is_valid_line(line, line_num))
		return false;
	
	sscanf(line, "%s", word);
	r_line = remove_whitespace(line) + strlen(word);
	
	/*Is there a label?*/
	if (word[strlen(word) - 1] == ':')
	{
		word[strlen(word) - 1] = '\0';
		/*validate the label*/
		if (!is_valid_label_decleration(word, line_num))
		{
			return false;
		}
		strcpy(label, word);
		r_line = remove_whitespace(r_line);
		sscanf(r_line, "%s", word);
		r_line += strlen(word);
	}
	
	/*Is it a directive?*/
	if (word[0] == '.')
	{
		return parse_directive(word+1, r_line, label, DC, line_num);
	}

	/*Then it must be an instruction*/
	if (label[0] != '\0')
		add_code_symbol(label, *IC);
	return parse_instruction(word, r_line, line_num, label, IC);
}


bool perform_first_transion(char* assembly)
{
	int IC = 0, DC = 0;
	char *line, *nextline;
	int line_num = 1;
	bool error = false;
	
	/*iterate line by line*/
	line = assembly;
	nextline = strstr(assembly, "\n");
	if (nextline)
	{
		*nextline = '\0';
		nextline++;
	}
	while (line)
	{
		if (line[0] != ';' && !is_empty_or_whitespace(line))
			error |= !parse_line(line, line_num, &IC, &DC);
		line_num++;
		line = nextline;
		if (nextline)
			nextline = strstr(nextline, "\n");
		if (nextline)
		{
			*nextline = '\0';
			nextline++;
		}
	}
	
	/*Check if a label declared as entry but does not exist in the file*/
	if (is_entry_not_found() || error)
		return false;
	
	/*increment the data symbols values by IC*/
	inc_data_values(IC);
	return true;
}