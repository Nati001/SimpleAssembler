#include "first_transition.h"
#include "second_transition.h"
#include "asm_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Open asm file  ('.as') and returns its content. Returns NULL if failed.*/
char* open_asm_file(char* file_name)
{
	int file_size = 0;
	FILE* file = NULL;
	char* asm = NULL;
	char* asm_file_name = malloc(strlen(file_name) + 4);

	strcpy(asm_file_name, file_name);
	strcat(asm_file_name, ".as");

	file = fopen(asm_file_name, "r");
	if (file)
	{
		/*Check the file size*/
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		/*Read from file*/
		asm = calloc(file_size + 1, 1);
		fread(asm, 1, file_size, file);
		fclose(file);
	}
	else
	{
		printf("Failed to open file: '%s'", asm_file_name);
	}

	free(asm_file_name);
	return asm;
}

/*Creates the object file '.ob'. Returns false if failed.*/
bool create_ob_file(char* file_name)
{
	int i, cur_addr = DEF_FIRST_ADDR;
	char line[22];
	int data_len, code_len;
	char** data = NULL;
	char** code = NULL;
	bool res = false;
	FILE* file = NULL;
	char* ob_file_name = malloc(strlen(file_name) + 4);

	strcpy(ob_file_name, file_name);
	strcat(ob_file_name, ".ob");

	file = fopen(ob_file_name, "w+");
	if (file)
	{
		code_len = get_code(&code);
		data_len = get_data(&data);

		/*Write the data and code length*/
		sprintf(line, "%d %d\n", code_len, data_len);
		fputs(line, file);

		/*Write the binary code*/
		for (i = 0; i < code_len; i++, cur_addr++)
		{
			sprintf(line, "%04d %.14s\n", cur_addr, code[i]);
			fputs(line, file);
		}

		/*Write the binary data*/
		for (i = 0; i < data_len; i++, cur_addr++)
		{
			sprintf(line, "%04d %.14s\n", cur_addr, data[i]);
			fputs(line, file);
		}

		fclose(file);
		res = true;
	}
	else
	{
		printf("Failed to create file: '%s'\n", ob_file_name);
	}

	free(ob_file_name);
	return res;
}

/*Create the entry file ('.ent'). Return false if failed*/
bool create_ent_file(char* file_name)
{
	char line[50];
	char* label;
	int val;
	bool res = false;
	FILE* file = NULL;
	char* ent_file_name = malloc(strlen(file_name) + 5);

	strcpy(ent_file_name, file_name);
	strcat(ent_file_name, ".ent");

	get_first_entry_symbol(&label, &val); 
	
	if (!label) /*if there's no entry labels - do nothing.*/
	{
		res = true;
	}
	else if (label && (file = fopen(ent_file_name, "w+")))
	{
		/*Write the labels info.*/
		while(label)
		{
			sprintf(line, "%s %d\n", label, val);
			fputs(line, file);
			get_next_entry_symbol(&label, &val);
		}

		fclose(file);
		res = true;
	}
	else
	{
		printf("Failed to create file: '%s'\n", ent_file_name);
	}

	free(ent_file_name);
	return res;
}

bool create_ext_file(char* file_name)
{
	char line[50];
	char* label;
	int ic;
	bool res = false;
	FILE* file = NULL;
	char* ext_file_name = malloc(strlen(file_name) + 5);

	strcpy(ext_file_name, file_name);
	strcat(ext_file_name, ".ext");
	
	get_first_ext(&label, &ic);

	if (!label)/* if there's no external labels - do nothing.*/
	{
		res = true;
	}
	else if ((file = fopen(ext_file_name, "w+")))
	{
		/*Write labels info*/
		while (label)
		{
			sprintf(line, "%s %d\n", label, ic);
			fputs(line, file);
			get_next_ext(&label, &ic);
		}

		fclose(file);
		res = true;
	}
	else
	{
		printf("Failed to create file: '%s'\n", ext_file_name);
	}

	free(ext_file_name);
	return res;
}

/*Creates the output file. If one fails -> doesn't continue. Return false if failed.*/
bool create_out_files(char* file_name)
{
	if (!create_ob_file(file_name))
		return false;

	if (!create_ent_file(file_name))
		return false;

	if (!create_ext_file(file_name))
		return false;
	
	return true;
}


/*Assemble a single file.*/
bool assemble_file(char* file_name)
{
	bool res = false;
	char* asm = NULL;
	asm = open_asm_file(file_name);
	if (asm && perform_first_transion(asm))
	{
		if (perform_second_transion())
		{
			if (create_out_files(file_name))
				res = true;
		}
	}

	/*Free internal structures between files.*/
	free_internal_structs();
	free(asm);
	return res;
}

int main(int argc, char *argv[])
{
	int i;

	if (argc <= 1)
	{
		printf("Usage: assembler <file1 name> <file2 name> ...\n");
		return -1;
	}

	for (i = 1; i < argc; i++)
	{
		printf("Compiling file '%s'\n", argv[i]);
		if (!assemble_file(argv[i]))
			printf("Failed to compile file '%s'\n", argv[i]);
	}

	printf("Done.\n");
	return 0;
}