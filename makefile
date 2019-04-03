all:
	bison --yacc --defines --output=parser.c parser.y
	flex --outfile=scanner.c scanner.l
	gcc symtable.h symtable.c quads.h quads.c Stack.h Stack.c scanner.c parser.c Function_stack.h Function_stack.c --output=Parser

clean:
	rm parser.h
	rm parser.c
	rm scanner.c
	rm Parser
