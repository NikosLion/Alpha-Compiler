
//stack for offset

typedef struct scopeSpace_stack{
  unsigned offset;
  struct scopeSpace_stack* next;
}scopeSpace_stack;

/////////////////////////////////
unsigned pop();
void push(unsigned offset);
void print_stack();
