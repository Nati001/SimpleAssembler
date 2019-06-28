#ifndef ASM_DATA_H_
#define ASM_DATA_H_

#include "utils.h"
#include "operators.h"

#define WORD_SIZE 14
#define LABEL_LEN 31
#define DEF_FIRST_ADDR 100
typedef enum { C_ABSOLUTE, C_EXTERN, C_RELOCATE } coding_type;

typedef struct
{
	int line_num;
	int ic;
	short op_code;
	addressing op_s_addr;
	addressing op_d_addr;
	char* op_s;
	char* op_d;
	addressing jwp_p1_addr;
	addressing jwp_p2_addr;
	char* jwp_p1;
	char* jwp_p2;
} instruction;

/*Add data to the binary data array. Each integer in 'data' is a binary word.*/
void add_data(directive_type type, int* data, int size);

/*Add a single binary wordr to the code array in location 'index'*/
void add_code(char* data, int index);

/*Returns the binary data array and its length*/
int get_data(char*** data);

/*Returns the binary code array and its length*/
int get_code(char*** code);

/*If symbol 'label' exists return its value, otherwise -1.*/
int is_symbol_exist(char* label);

/*Add a symbol of type data to the symbol table*/
void add_data_symbol(char* label, int value);

/*Increase the value of all the data symbols by 'val'*/
void inc_data_values(int val);

/*Add a symbol of type code to the symbol table*/
void add_code_symbol(char* label, int value);

/*Add a symbol of type extern to the symbol table*/
void add_extern_symbol(char* label);

/*Return true if the symbol 'label' is stored in the symbol tabel and if it's an external symbol.*/
bool is_extern_symbol(char* label);

/*Get the first entry symbol in the data table*/
void get_first_entry_symbol(char** label, int* value);
/*Iterate through the entry symbols. One must call 'get_first_entry_symbol' before.*/
void get_next_entry_symbol(char** label, int* value);

/*Save a symbol as entry.*/
void save_entry_label(char* label, int line_num);

/*Get the first entry label not yet stored in the symbol table*/
void get_first_entry(char** label, int* line_num);
/*Iterate through the entry label. One must call 'get_first_entry' before.*/
void get_next_entry(char** label, int* line_num);

/*Add an instruction info to the internal data structure*/
void add_instruction(instruction* inst);

/*Get the first instruction stored*/
instruction* get_first_instr();
/*Iterate through the stored instructions. One must call 'get_first_instr' before.*/
instruction* get_next_instr();

/*Store the ic where and external symbol used*/
void ext_sym_use(char* label, int ic);

/*Get the first usage of an external symbol*/
void get_first_ext(char** label, int* ic);
/*Iterate through the usages of external symbols. One must call 'get_first_ext' before.*/
void get_next_ext(char** label, int* ic);

/*Free all the internal structs and set the globals to zero. */
void free_internal_structs();


#endif