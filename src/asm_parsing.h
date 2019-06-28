#ifndef ASM_VALID_H_
#define ASM_VALID_H_

#include "utils.h"
#include "asm_data.h"
#include "operators.h"

/*Checks if 'label' is a valid label name for decleration and print error if not.*/
bool is_valid_label_decleration(char* label, int line_num);

/*Parses and checks the directive statement and extract the data into 'data' if relevant. Prints the error if failed.*/
bool is_valid_directive_params(directive_type type, char* params, int line_num, int** data, int* data_size);

/*Parses and checks the instruction statement and returns the parsed info in 'inst'. Prints error if failed.*/
bool is_valid_instruction(asm_operator* op, char* params, instruction* inst, int line_numx);

/*Checks if an entry label was not declared. Prints errors if so.*/
bool is_entry_not_found();

/*Returns the label's value if exists and -1 otherwise. Prints error if label was not found.*/
int validate_and_find_label(char* label, int line_num);

/*Return the operator info by name if found and NULL otherwise. Prints error if not found.*/
asm_operator* find_valid_op(char* op_name, int line_num);

/*Check the length limitations of an assembly line and prints error if not valid.*/
bool is_valid_line(char* line, int line_num);

#endif