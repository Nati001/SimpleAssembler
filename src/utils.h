#ifndef UTILS_H_
#define UTILS_H_


typedef enum { false, true } bool;

/*Converts an integer to asm wierd binary in 'size' bits represented as characters ('.' and '/').*/
void int_to_binary(int val, char* bin, int size);

/*Returns how much times character 'c' apears in string 'str'.*/
int count_char_in_str(char* str, char c);

/*Returns true is string 'str' is empty or contains only white-space characters.*/
bool is_empty_or_whitespace(char* str);

/*Removes the trailing abd leading whitespace parameters. 
The function modify 'str' and the return pointer points to a character in str itself*/
char* remove_whitespace(char* str);

/*Returns true if the character 'c' is a code ascii of a digit.*/
bool is_letter(char c);

/*Returns true if the character 'c' is a code ascii of a letter.*/
bool is_digit(char c);

/*Returns true if the string 'str' is a string representation of a number.*/
bool is_number(char* str);

#endif