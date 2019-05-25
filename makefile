all:
	bison --yacc --defines --output=parser.c Parser.y
	flex --outfile=scanner.c Scanner.l
	gcc  Symtable.h Symtable.c Quads.h Quads.c Offset_stack.h Offset_stack.c Function_stack.h Function_stack.c Table_queue.h Table_queue.c Vm_args.h Vm_args.c A_vm.h A_vm.c Main.c scanner.c parser.c --output=Parser -lm

clean:
	rm parser.h
	rm parser.c
	rm scanner.c
	rm Parser
	rm output
