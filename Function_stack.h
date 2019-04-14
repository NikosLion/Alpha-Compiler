
//stack for funcname

typedef struct Function_stack{
  char *name;
  int label;
  struct Function_stack* next;
}Function_stack;

//////////////////////////////

Function_stack* f_pop();
void f_push(char *name,int label);
void f_print_stack();
