#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <math.h> 


void int_to_binary(int val, char* bin, int size)
{
	int n = 1 << (size-1);
	int i;
	int x;
	
	/*Iterate the int bits from letf to right*/
	for (i = 0; i < size; i++)
	{
		x = (n >> i);
		if ((val & x) == x) /*square(x) bit is set*/
			bin[i] = '/';
		else
			bin[i] = '.';
	}
}

bool is_whitespace(char c)
{
	/*Consider \n and \r as whitespace since they have the same affect as ' ' or \t. */
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

bool is_empty_or_whitespace(char* str)
{
	char* it = str;

	while (*it)
	{
		if (!is_whitespace(*it))/*One none whitespace character is enough to return false*/
			return false;
		it++;
	}
	return true;
}

char* remove_whitespace(char* str)
{
	int i;
	char* ret = str;
	
	/*"Remove" leading whitespace chars.*/
	while (*ret && is_whitespace(*ret))
		ret++;
	
	/*Remove trailing whaitespace chars.*/
	for (i = strlen(str) - 1; (i >= 0) && is_whitespace(str[i]) ; i--)
	{
		str[i] = '\0';
	}

	return ret;
}

bool is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

bool is_number(char* str)
{
	int i;

	/*The first character may be a digit or a sign.*/
	if (!str || (str[0] != '-' && str[0] != '+' && !is_digit(str[0])))
		return false;
	
	/*The rest of the charcters must be digits.*/
	for (i = 1; i < strlen(str); i++)
		if (!is_digit(str[i]))
			return false;
	
	/*A number in our asm must be at most 11 bits long -> therefore smaller then 2^11.*/
	if ((atoi(str) >= (2 << 11)) || (atoi(str) <= 0-(2 << 11)))
		return false;

	return true;
}

int count_char_in_str(char* str, char c)
{
	int i;
	int count = 0;

	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] == c)
			count++;
	}
	return count;
}
