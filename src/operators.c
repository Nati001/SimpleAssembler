#include <stdio.h>
#include "operators.h"
#include <string.h>

#define STR_DATA "data"
#define STR_STRING "string"
#define STR_ENTRY "entry"
#define STR_EXTERN "extern"

asm_operator ops[] = 	{{"mov",{IMMEDIATE, DIRECT, REGISTER,-1}, {DIRECT, REGISTER,-1,-1},0},
					{"cmp",{IMMEDIATE,DIRECT , REGISTER,-1},{IMMEDIATE,DIRECT , REGISTER,-1},1},
					{"add",{IMMEDIATE,DIRECT , REGISTER,-1},{DIRECT,REGISTER,-1,-1},2},
				  	{"sub",{IMMEDIATE,DIRECT , REGISTER,-1},{DIRECT,REGISTER,-1,-1},3},
				  	{"not",{-1,-1,-1,-1},{DIRECT,REGISTER,-1,-1},4},
				  	{"clr",{-1,-1,-1,-1},{DIRECT,REGISTER,-1,-1},5},
				  	{"lea",{DIRECT,-1,-1,-1},{DIRECT,REGISTER,-1,-1},6},
				  	{"inc",{-1,-1,-1,-1},{DIRECT,REGISTER,-1,-1},7},
				  	{"dec",{-1,-1,-1,-1},{DIRECT,REGISTER,-1,-1},8},
				  	{"jmp",{-1,-1,-1,-1},{DIRECT,JAMP_WITH_PRAMTER,REGISTER,-1},9},
				  	{"bne",{-1,-1,-1,-1},{DIRECT,JAMP_WITH_PRAMTER,REGISTER,-1},10},
				  	{"red",{-1,-1,-1,-1},{DIRECT,REGISTER,-1,-1},11},
				  	{"prn",{-1,-1,-1,-1},{IMMEDIATE,DIRECT,REGISTER,-1},12},
				  	{"jsr",{-1,-1,-1,-1},{DIRECT,JAMP_WITH_PRAMTER,REGISTER,-1},13},
				  	{"rts",{-1,-1,-1,-1},{-1,-1,-1,-1},14},
				  	{"stop", {-1,-1,-1,-1},{-1,-1,-1,-1},15}};

/*Allowed addressing methods for the parameters of Jump with Parameters addressing method.*/
addressing jwp_addressing[] = { IMMEDIATE, DIRECT, REGISTER, NONE };  

/*Gets an operator name and returns all the indo about it (listed in asm_operator struct).*/
asm_operator* find_op(char* op)
{
	int i;
	for(i= 0 ; i < sizeof(ops)/sizeof(asm_operator); i++)
	{
		if(strcmp(ops[i].name, op)== 0)
			return &ops[i];
	}
	return NULL;
}

/*Gets a directive string and returns the corresponting element in directive_type enum.*/
directive_type get_directive_type(char* directive)
{
	if (strcmp(directive, STR_DATA) == 0)
		return D_DATA;
	if (strcmp(directive, STR_STRING) == 0)
		return D_STRING;
	if (strcmp(directive, STR_ENTRY) == 0)
		return D_ENTRY;
	if (strcmp(directive, STR_EXTERN) == 0)
		return D_EXTERN;
	
	return D_NONE;
}