all:
	bison --yacc --defines --output=parser.c parser.y
	flex --outfile=scanner.c scanner.l
	gcc symtable.h symtable.c scanner.c parser.c --output=Parser

clean:
	rm parser.h
	rm parser.c
	rm scanner.c
	rm Parser
	
