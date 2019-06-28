assembler: asm_data.o asm_parsing.o assembler.o first_transition.o operators.o second_transition.o utils.o 
	gcc -o $@ $^

%.o:%.c
	gcc -c -g -Wall -ansi -pedantic $<
	
clean:
	rm *.o *~