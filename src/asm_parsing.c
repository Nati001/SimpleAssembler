#include "asm_parsing.h"
#include "asm_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Print formated error*/
#define PRINT_ERR(line_num, str) fprintf(stderr, "Error in line %d: %s\n", line_num, str)

/*Get the number of a register. Returns -1 if the string is not a valid gerister.*/
int get_register_number(char* reg)
{
	int reg_num;
	if (!reg || reg[0] != 'r')
		return -1;

	reg_num = atoi(&reg[1]);
	return (reg_num >= 1 && reg_num <= 6) ? reg_num : -1;
}

/*Check if 'label' is a valid name for a label.*/
bool is_valid_label_name(char* label)
{
	int i;
	
	if (strlen(label) > LABEL_LEN)
		return false;

	if (!is_letter(label[0]))
		return false;

	for (i = 1; i < strlen(label); i++)
		if (!is_letter(label[i]) && !is_digit(label[i]))
			return false;

	/*'label' can't be a saved word.*/
	if (get_register_number(label) != -1 || get_directive_type(label) != D_NONE || find_op(label) != NULL)
		return false;

	return true;
}

bool is_valid_label_decleration(char* label, int line_num)
{
	char *format, *l = remove_whitespace(label);
	char message[70] = "";

	/*First of all the name should be valid*/
	if (!is_valid_label_name(l))
	{
		format = "Invalid label name '%s' in declaration.";
		if (strlen(format) + strlen(l) + 1 > sizeof(message))
			l[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, l);
		PRINT_ERR(line_num, message);
		return false;
	}

	/*If exists - the label cannot be declared again.*/
	if (is_symbol_exist(l) != -1)
	{
		format = "Label '%s' already declared.";
		if (strlen(format) + strlen(l) + 1 > sizeof(message))
			l[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, l);
		PRINT_ERR(line_num, message);
		return false;
	}

	return true;
}

/*Parse a string representing a number array and put it in 'data'. Return false if parsing failed.*/
bool get_valid_num_arr(char* str, int** data, int* data_size)
{
	char *num, *rest;
	int i = 0;

	*data_size = count_char_in_str(str, ',') + 1;
	*data = malloc(*data_size * sizeof(int));
	num = str;
	rest = strstr(str, ",");
	if (rest)
	{
		*rest = '\0';
		rest++;
	}
	while(num)
	{
		num = remove_whitespace(num);
		if (!is_number(num)) /*Token must be a valid asm number*/
		{
			free(*data);
			*data = NULL;
			return false;
		}
		
		/*Store the number*/
		(*data)[i] = atoi(num); 
		i++;
		num = rest;
		if (rest)
			rest = strstr(rest, ",");
		if (rest)
		{
			*rest = '\0';
			rest++;
		}
	}
	return true;
}

/*Parse a string representing an asm string and put it in 'data', each ascii code in its own int entry. 
Empty string is a valid string. Return false if parsing failed.*/
bool get_valid_string(char* str, int** data, int* data_size)
{
	int i;

	str = remove_whitespace(str);
	/*String should be enclosed with quotation marks.*/
	if (str[0] != '\"' || str[strlen(str) - 1] != '\"')
		return false;

	*data_size = strlen(str) - 1;
	*data = malloc(*data_size * sizeof(int));
	/*store the data*/
	for (i = 0; i < *data_size - 1; i++)
	{
		(*data)[i] = str[i + 1];
	}
	(*data)[i] = '\0';
	return true;
}

bool is_valid_directive_params(directive_type type, char* params, int line_num, int** data, int* data_size)
{
	char *format, message[70] = "";
	char* p = remove_whitespace(params);

	switch (type)
	{
	case D_DATA:
		if (!get_valid_num_arr(p, data, data_size))
		{
			sprintf(message, "Invalid numbers array in declaration.");
			PRINT_ERR(line_num, message);
			return false;
		}
		return true;

	case D_STRING:
		if (!get_valid_string(p, data, data_size))
		{
			format = "Invalid string '%s'.";
			if (strlen(format) + strlen(p) + 1 > sizeof(message))
				p[sizeof(message) - strlen(format) - 1] = '\0';
			sprintf(message, format, p);
			PRINT_ERR(line_num, message);
			return false;
		}
		return true;

	case D_ENTRY:
	case D_EXTERN:
		if (!is_valid_label_name(p))
		{
			format = "Invalid label name '%s'.";
			if (strlen(format) + strlen(p) + 1 > sizeof(message))
				p[sizeof(message) - strlen(format) - 1] = '\0';
			sprintf(message, format, p);
			PRINT_ERR(line_num, message);
			return false;
		}
		return true;
	default:
		sprintf(message, "Unknown directive.");
		PRINT_ERR(line_num, message);
		return false;

		return false;
	}
}

/*Parse and validate instruction with no operands and put the info in 'inst'. Return false if invalid.*/
bool check_inst_wo_operands(asm_operator* op, instruction* inst, int line_num)
{
	char *format;
	char message[70] = "";

	if (op->op_d_addr[0] != NONE)
	{
		format = "Operator '%s' should get more operands.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op->name);
		PRINT_ERR(line_num, message);
		return false;
	}
	return true;
}

/*Parse and validate a single operand. Return false if invalid.*/
bool check_operand(addressing supported_addressing[4], char* op, addressing* cur_addr, char** op_to_save, int line_num)
{
	int ok = false;
	int i;
	char *format;
	char message[70] = "";
	int reg_num = -1;
	char* op_n = remove_whitespace(op);
	switch (op_n[0])
	{
	case '#': /*Then addresing is immediate*/
		if (is_number(op_n + 1))
		{
			*op_to_save = malloc(strlen(op_n));
			strcpy(*op_to_save, op_n + 1);
			*cur_addr = IMMEDIATE;
			break;
		}
		else
		{
			format = "Operand '%s' must be a valid number smaller then 2048.";
			if (strlen(format) + strlen(op_n) + 1 > sizeof(message))
				op_n[sizeof(message) - strlen(format) - 1] = '\0';
			sprintf(message, format, op_n);
			PRINT_ERR(line_num, message);
			return false;
		}
	case 'r': /*If it's a valid register it's register addressing - but may be direct too, hence no break if invalid.*/
		if ((reg_num = get_register_number(op_n)) != -1)
		{
			*op_to_save = malloc(2);
			sprintf(*op_to_save, "%d", reg_num);
			*cur_addr = REGISTER;
			break;
		}
	default: /*Then it should be a label*/
		if (is_valid_label_name(op_n))
		{
			*op_to_save = malloc(strlen(op_n)+1);
			strcpy(*op_to_save, op_n);
			*cur_addr = DIRECT;
			break;
		}
		else
		{
			format = "The operand '%s' is not a valid label.";
			if (strlen(format) + strlen(op_n) + 1 > sizeof(message))
				op_n[sizeof(message) - strlen(format) - 1] = '\0';
			sprintf(message, format, op_n);
			PRINT_ERR(line_num, message);
			return false;
		}
	}

	/*Does the addresing that we find is supported by the operand?*/
	for (i = 0; i < MAX_ADDR; i++)
	{
		if (*cur_addr == supported_addressing[i])
			ok = true;
	}

	if (!ok)
	{
		format = "The operand '%s' doesn't have a valid addressing method.";
		if (strlen(format) + strlen(op_n) + 1 > sizeof(message))
			op_n[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op_n);
		PRINT_ERR(line_num, message);
	}

	return ok;
}

/*Check jump with parameters addressing method*/
bool check_jwp(asm_operator* op, char* params, instruction* inst, int line_num)
{
	char *format;
	char message[70] = "";
	bool is_jwp_allowed = false;
	int i;
	char *operand, *jwp_param1, *jwp_param2, *rest;

	/*Is jwp supported by the operator?*/
	for (i = 0; i < sizeof(op->op_d_addr) / sizeof(op->op_d_addr[0]); i++)
		if (op->op_d_addr[i] == JAMP_WITH_PRAMTER)
			is_jwp_allowed = true;

	if (!is_jwp_allowed)
	{
		format = "Operator '%s' doesn't support jump with parameters addressing.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op->name);
		PRINT_ERR(line_num, message);
		return false;
	}

	inst->op_d_addr = JAMP_WITH_PRAMTER;

	operand = params;
	rest = strstr(params, "(");
	*rest = '\0';
	rest++;

	/*In jwp there's can be only destination operand.*/
	if (strstr(operand, ","))
	{
		format = "Operator '%s' can get only one parameter.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op->name);
		PRINT_ERR(line_num, message);
		return false;
	}

	operand = remove_whitespace(operand);
	/*Operand must be a label*/
	if (!is_valid_label_name(operand))
	{
		format = "The operand '%s' is not a valid label.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, operand);
		PRINT_ERR(line_num, message);
		return false;
	}

	inst->op_d = malloc(strlen(operand) + 1);
	strcpy(inst->op_d, operand);

	/*Check the syntax*/
	if (rest[strlen(rest)-1] != ')')
	{
		sprintf(message, "Missing character ')'.");
		PRINT_ERR(line_num, message);
		return false;
	}

	rest[strlen(rest) - 1] = '\0';
	jwp_param1 = rest;
	rest = strstr(rest, ",");
	if (!rest)
	{
		sprintf(message, "Jump with parameters addressing must have 2 parameters.");
		PRINT_ERR(line_num, message);
		return false;
	}

	*rest = '\0';
	jwp_param2 = rest+1;

	/*Check the parametes themselves*/
	if (!check_operand(jwp_addressing, jwp_param1, &inst->jwp_p1_addr, &inst->jwp_p1, line_num))
		return false;
	
	if (!check_operand(jwp_addressing, jwp_param2, &inst->jwp_p2_addr, &inst->jwp_p2, line_num))
		return false;

	return true;
}

/*Parse and check operator with 2 operands. Return bool if invalid.*/
bool check_2_op(asm_operator* op, char* params, instruction* inst, int line_num)
{
	char* format;
	char message[70] = "";
	char *op_s, *op_d;

	/*The operator should support 2 operands.*/
	if (op->op_s_addr[0] == NONE )
	{
		format = "Operator '%s' do not recive 2 operands.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op->name);
		PRINT_ERR(line_num, message);
		return false;
	}

	op_s = params;
	op_d = strstr(params, ",");
	*op_d = '\0';
	op_d++;

	/*Check the operands themselves*/
	if (!check_operand(op->op_s_addr, op_s, &inst->op_s_addr, &inst->op_s, line_num))
		return false;

	if (!check_operand(op->op_d_addr, op_d, &inst->op_d_addr, &inst->op_d, line_num))
		return false;


	return true;
}

bool check_1_op(asm_operator* op, char* params, instruction* inst, int line_num)
{
	char* format;
	char message[70] = "";

	/*The operator should support 1 operand*/
	if (op->op_s_addr[0] != NONE || op->op_d_addr[0] == NONE)
	{
		format = "Operator '%s' does not recive one operand.";
		if (strlen(format) + strlen(op->name) + 1 > sizeof(message))
			op->name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op->name);
		PRINT_ERR(line_num, message);
		return false;
	}

	/*Check the operand itself.*/
	if (!check_operand(op->op_d_addr, params, &inst->op_d_addr, &inst->op_d, line_num))
		return false;

	return true;
}

bool is_valid_instruction(asm_operator* op, char* params, instruction* inst, int line_num)
{
	inst->op_code = op->op_code;
	if (is_empty_or_whitespace(params)) /*No operands at all*/
		return check_inst_wo_operands(op, inst, line_num);

	if (strstr(params, "(")) /* It's a jwp*/
		return check_jwp(op, params, inst, line_num);

	if (strstr(params, ",")) /* It has 2 operands*/
		return check_2_op(op, params, inst, line_num);

	return check_1_op(op, params, inst, line_num); /*Otherwise 1 operand*/
}

bool is_entry_not_found()
{
	char *label, *format;
	int line_num;
	char message[70] = "";
	
	get_first_entry(&label, &line_num);
	if (!label) /*all entries where declared and hence deleted.*/
		return false;

	/*Iterate through the internal data to print error for each one.*/
	while (label)
	{
		format = "Label '%s' declaration not found.";
		if (strlen(format) + strlen(label) + 1 > sizeof(message))
			label[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, label);
		PRINT_ERR(line_num, message);
		get_next_entry(&label, &line_num);
	}
	return true;
}

int validate_and_find_label(char* label, int line_num)
{
	char* format;
	char message[70] = "";
	
	int label_val = is_symbol_exist(label);
	if (label_val == -1)
	{
		format = "Label '%s' declaration not found.";
		if (strlen(format) + strlen(label) + 1 > sizeof(message))
			label[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, label);
		PRINT_ERR(line_num, message);
	}
	return label_val;
}

asm_operator* find_valid_op(char* op_name, int line_num)
{
	char* format;
	char message[70] = "";

	asm_operator* op = find_op(op_name);
	if (!op)
	{
		format = "Invalid operator '%s'.";
		if (strlen(format) + strlen(op_name) + 1 > sizeof(message))
			op_name[sizeof(message) - strlen(format) - 1] = '\0';
		sprintf(message, format, op_name);
		PRINT_ERR(line_num, message);
	}
	return op;
}

bool is_valid_line(char* line, int line_num)
{
	char message[70] = "";
	
	/*Assembly line can't be longer that 80 characters.*/
	if (strlen(line) > 80)
	{
		sprintf(message, "Line too long.");
		PRINT_ERR(line_num, message);
		return false;
	}
	
	return true;
}