#ifndef OPERATOR_H_
#define OPERATOR_H_

typedef enum { D_DATA, D_STRING, D_ENTRY, D_EXTERN, D_NONE } directive_type;
typedef enum { IMMEDIATE, DIRECT, JAMP_WITH_PRAMTER, REGISTER, MAX_ADDR, NONE = -1 } addressing;

typedef struct
{
	char name[5];
	addressing op_s_addr[4];
	addressing op_d_addr[4];
	short op_code;
} asm_operator;

extern addressing jwp_addressing[];  

/*Gets an operator name and returns all the indo about it (listed in asm_operator struct).*/
asm_operator* find_op(char* op);

/*Gets a directive string and returns the corresponting element in directive_type enum.*/
directive_type get_directive_type(char* directive);

#endif
