#include "asm_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {DATA =1 ,EXTRN, CODE = 4  ,ENTRY = 8} sym_type;

typedef struct _entry_list{
	char name[32];
	int line_num;
	struct _entry_list* prev;
	struct _entry_list* next;
} entry_list;

entry_list* head_entry = NULL;

/*Add a label to the list of entry label not yet declared*/
void add_entry(char* name, int line_num)
{
	/*Initialize the entry struct*/
	entry_list* temp_entry=(entry_list*) malloc(sizeof(entry_list));
	strcpy(temp_entry->name, name);
	temp_entry->line_num = line_num;
	
	/*Store it in the list*/
	temp_entry->prev = NULL;
	temp_entry->next = head_entry;
	if (head_entry)
		head_entry->prev = temp_entry;
	head_entry = temp_entry;
}

/*Find a label in the list of entry label not yet declared*/
entry_list* find_entry(char name_entry[]) 
{
	entry_list* temp = head_entry; 
	while(temp != NULL) 
	{
		if(strcmp(temp->name, name_entry) == 0)
		{
			return	temp; 
		}
		temp =temp->next;
	 }
     return NULL;
}

/*Delete a label from the list of entry label not yet declared*/
void delete_entry(entry_list* to_delete)
{
	if (head_entry == to_delete)
		head_entry = to_delete->next;
	if (to_delete->prev != NULL)
		to_delete->prev->next =to_delete->next;
	if (to_delete->next != NULL)
		to_delete->next->prev = to_delete->prev;
    free(to_delete);
	to_delete = NULL;
}

/*Iteration functions for iterating through the list of entry tables not yet declared.*/
entry_list* entry_it = NULL;
void get_first_entry(char** label, int* line_num)
{
	if (head_entry)
	{
		*label = head_entry->name;
		*line_num = head_entry->line_num;
		entry_it = head_entry->next;
	}
	else
	{
		*label = NULL;
		*line_num = -1;
	}
}

void get_next_entry(char** label, int* line_num)
{
	if (entry_it)
	{
		*label = entry_it->name;
		*line_num = entry_it->line_num;
		entry_it = entry_it->next;
	}
	else
	{
		*label = NULL;
		*line_num = -1;
	}
}

/*Free the list of entry not yet declared*/
void free_entry_list()
{
	entry_it = head_entry;

	while (entry_it)
	{
		head_entry = entry_it->next;
		free(entry_it);
		entry_it = head_entry;
	}
	head_entry = NULL;
	entry_it = NULL;
}

typedef struct{
	char name[32];
	int val;
	sym_type type;
} symbols_table_entry;  
			
#define DEF_SIZE 10

symbols_table_entry *s_t;

int st_length = 0;
int st_capacity = 0;

/*Add a symbol to the symbol table*/
void add_symbol(char* label, int value, sym_type type)
{
	entry_list* e;

	/*Allocate more space to the symbo table if needed*/
	if (st_length == st_capacity)
	{
		s_t = (symbols_table_entry*)realloc(s_t, st_capacity + sizeof(symbols_table_entry)*DEF_SIZE);
		st_capacity += DEF_SIZE;
	}

	s_t[st_length].val = value + DEF_FIRST_ADDR; /*DEF_FIRST_ADDR should be added to all the values*/
	strcpy(s_t[st_length].name, label);
	s_t[st_length].type = type;
	/*If the label is in the list of entry labels not yet declared, 
	it should be set as entry label and then erased from the list.*/
	e = find_entry(s_t[st_length].name); 
	if (e)
	{
		delete_entry(e);
		s_t[st_length].type |= ENTRY;
	}
	st_length++;
}

void add_data_symbol(char* label, int value)
{
	add_symbol(label, value, DATA);
}

void add_code_symbol(char* label, int value)
{
	add_symbol(label, value, CODE);
}

void add_extern_symbol(char* label)
{
	/*Allocate more memory to the symbol table if needed*/
	if(st_length == st_capacity)
   	{
		s_t = (symbols_table_entry*)realloc(s_t, st_capacity + sizeof(symbols_table_entry)*DEF_SIZE);
		st_capacity +=DEF_SIZE;
   	}
	
	strcpy(s_t[st_length].name,label);
	s_t[st_length].val = 0; /*extern symbols don't have value - will be set by the linker.*/
	s_t[st_length].type= EXTRN ;
	st_length++;
}

/*Find a symbol in the symbol table - return NULL if not found.*/
symbols_table_entry* find_symbol(char* label)
{
	int i;
	for (i=0; i<st_length; i++)
	{
		if(strcmp(s_t[i].name, label) == 0)
		{
		    return &s_t[i]; 
		}
	 }
     return NULL;
}

int is_symbol_exist(char* label) 
{
	symbols_table_entry* e = find_symbol(label);
	return e? e->val: -1;
}

bool is_extern_symbol(char* label)
{
	symbols_table_entry* e = find_symbol(label);
	return e && ((e->type & EXTRN) == EXTRN);
}

/*If the label is in the symbol table already - mark it as entry. Otherwise store it for later inspection.*/
void save_entry_label(char* label, int line_num)
{
	symbols_table_entry* sym = find_symbol(label);

	if (sym)
	{
		sym->type |= ENTRY;
	}
	else
	{
		add_entry(label, line_num);
	}
}

void inc_data_values(int val)
{
	int i;
	for (i = 0; i<st_length; i++)
	{
		if ((s_t[i].type & DATA) == DATA)
		{
			s_t[i].val += val;
		}
	}
}


/*Iteration functions to iterate through the symbols marked as entry in the symbol table.*/
int entry_symbol_idx;
void get_first_entry_symbol(char** label, int* value)
{
	*label = NULL;
	*value = -1; 
	for (entry_symbol_idx = 0; entry_symbol_idx < st_length; entry_symbol_idx++)
	{
		if ((s_t[entry_symbol_idx].type & ENTRY) == ENTRY)
		{
			*label = s_t[entry_symbol_idx].name;
			*value = s_t[entry_symbol_idx].val;
			break;
		}
	}
	entry_symbol_idx++;
}

void get_next_entry_symbol(char** label, int* value)
{
	*label = NULL;
	*value = -1;
	for (; entry_symbol_idx < st_length; entry_symbol_idx++)
	{
		if ((s_t[entry_symbol_idx].type & ENTRY) == ENTRY)
		{
			*label = s_t[entry_symbol_idx].name;
			*value = s_t[entry_symbol_idx].val;
			break;
		}
	}
	entry_symbol_idx++;
}

/*Free the symbol table*/
void free_symbol_table()
{
	free(s_t);
	s_t = NULL;
	st_length = 0;
	st_capacity = 0;
}

char** bin_data;
int data_len;
int data_capacity;

char** bin_code;
int code_len;
int code_capacity;

void add_data(directive_type type, int* data, int size)
{
	int i = 0;
	
	/*Allocate more memory if needed*/
	if (data_len + size >= data_capacity)
	{
		bin_data = realloc(bin_data, (data_capacity + DEF_SIZE + size) * sizeof(char*));
		data_capacity += DEF_SIZE + size;
	}

	/*Add the data*/
	for (i = 0; i < size; i++, data_len++)
	{
		bin_data[data_len] = malloc(WORD_SIZE * sizeof(char));
		int_to_binary(data[i], bin_data[data_len], WORD_SIZE);
	}
}

void add_code(char* data, int index)
{
	/*Allocate more memory if needed*/
	if (index >= code_capacity)
	{
		bin_code = realloc(bin_code, (code_capacity + DEF_SIZE) * sizeof(char*));
		/*memory must be set to zero - in case of error entry may not be initialize and we need to know wether to free them or not.*/
		memset(bin_code + code_capacity, 0, DEF_SIZE * sizeof(char*));
		code_capacity += DEF_SIZE;
	}

	/*Add the data*/
	bin_code[index] = malloc(WORD_SIZE * sizeof(char));
	memcpy(bin_code[index], data, WORD_SIZE);
	if (index > code_len)
		code_len = index + 1;
}

int get_data(char*** data)
{
	*data = bin_data;
	return data_len;
}

int get_code(char*** code)
{
	*code = bin_code;
	return code_len;
}

/*Free data array*/
void free_data()
{
	int i;
	for (i = 0; i < data_len; i++)
		free(bin_data[i]);
	free(bin_data);
	bin_data = NULL;
	data_len = 0;
	data_capacity = 0;
}

/*Free code array*/
void free_code()
{
	int i;
	for (i = 0; i < code_len; i++)
		free(bin_code[i]);
	free(bin_code);
	bin_code = NULL;
	code_len = 0;
	code_capacity = 0;
}

typedef struct _inst_entry{
	instruction* inst;
	struct _inst_entry* next;
} inst_entry;

inst_entry* inst_head;
inst_entry* inst_it;

void add_instruction(instruction* inst)
{
	inst_entry* temp = (inst_entry*)malloc(sizeof(inst_entry));
	temp->inst = inst;
	temp->next = inst_head;
	inst_head = temp;
}

/*Iteration functions to iterate through the instructions.*/
instruction* get_first_instr()
{
	if (inst_head)
	{
		inst_it = inst_head;
		return inst_it->inst;
	}
	return NULL;
}

instruction* get_next_instr()
{
	if (inst_it->next)
	{
		inst_it = inst_it->next;
		return inst_it->inst;
	}
	return NULL;
}

/*Free the instruction list*/
void free_instruction_list()
{
	inst_it = inst_head;

	while (inst_it)
	{
		inst_head = inst_it->next;
		if (inst_it->inst->jwp_p1)
			free(inst_it->inst->jwp_p1);
		if(inst_it->inst->jwp_p2)
			free(inst_it->inst->jwp_p2);
		if (inst_it->inst->op_d)
			free(inst_it->inst->op_d);
		if (inst_it->inst->op_s)
			free(inst_it->inst->op_s);
		free(inst_it->inst);
		free(inst_it);
		inst_it = inst_head;
	}
	inst_head = NULL;
	inst_it = NULL;
}

typedef struct _ext_entry {
	char label[32];
	int ic;
	struct _ext_entry* next;
} ext_entry;

ext_entry* ext_head;
ext_entry* ext_it;

void ext_sym_use(char* label, int ic)
{
	ext_entry* temp = (ext_entry*)malloc(sizeof(ext_entry));
	strcpy(temp->label, label);
	temp->ic = ic + DEF_FIRST_ADDR;
	temp->next = ext_head;
	ext_head = temp;
}

/*Iteration functoins to iterate through the usages of external symbols*/
void get_first_ext(char** label, int* ic)
{
	if (ext_head)
	{
		*label = ext_head->label;
		*ic = ext_head->ic;
		ext_it = ext_head->next;
	}
	else
	{
		*label = NULL;
	}
}

void get_next_ext(char** label, int* ic)
{
	if (ext_it)
	{
		*label = ext_it->label;
		*ic = ext_it->ic;
		ext_it = ext_it->next;
	}
	else
	{
		*label = NULL;
	}
}

/*Free the external usages list*/
void free_ext_list()
{
	ext_it = ext_head;

	while (ext_it)
	{
		ext_head = ext_it->next;
		free(ext_it);
		ext_it = ext_head;
	}
	ext_head = NULL;
	ext_it = NULL;
}

void free_internal_structs()
{
	free_ext_list();
	free_instruction_list();
	free_data();
	free_code();
	free_symbol_table();
	free_entry_list();
}
