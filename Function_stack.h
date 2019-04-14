
//stack for funcname

typedef struct Function_stack{
  char *name;
  struct Function_stack* next;
}Function_stack;

//////////////////////////////

char* f_pop();
void f_push(char *name);
void f_print_stack();
