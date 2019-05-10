#include "Table_queue.h"



enum vmopcode{
  assign_v,       add_v,          sub_v,
  mul_v,          div_v,          mod_v,
  uminus_v,       and_v,          or_v,
  not_v,          jeq_v,          jne_v,
  jle_v,          jge_v,          jlt_v,
  jgt_v,          call_v,         pusharg_v,
  funcenter_v,    funcexit_v,     newtable_v,
  tablegetelem_v, tablesetelem_v, nop_v
};

enum vmarg_t{
  label_a=0,
  global_a=1,
  formal_a=2,
  local_a=3,
  number_a=4,
  string_a=5,
  bool_a=6,
  nil_a=7,
  userfunc_a=8,
  libfunc_a=9,
  retval_a=10
};



typedef struct vmarg{
  enum vmarg_t type;
  unsigned val;
}vmarg;

typedef struct instruction{
  enum vmopcode  opcode;
  vmarg     result;
  vmarg     arg1;
  vmarg     arg2;
  unsigned  srcLine;
}instruction;

typedef struct userfunc{
  unsigned  address;
  unsigned  localSize;
  char*     id;
}userFunc;

typedef void (*generator_func_t)(quad*);

extern void generate_ADD(quad*);
extern void generate_SUB(quad*);
extern void generate_MUL(quad*);
extern void generate_DIV(quad*);
extern void generate_MOD(quad*);
extern void generate_NEWTABLE(quad*);
extern void generate_TABLEGETELEM(quad*);
extern void generate_TABLESETELEM(quad*);
extern void generate_ASSIGN(quad*);
extern void generate_NOP(quad*);
extern void generate_JUMP(quad*);
extern void generate_IF_EQ(quad*);
extern void generate_IF_NOTEQ(quad*);
extern void generate_IF_GREATER(quad*);
extern void generate_IF_GREATEREQ(quad*);
extern void generate_IF_LESS(quad*);
extern void generate_IF_LESSEQ(quad*);
extern void generate_NOT(quad*);
extern void generate_PARAM(quad*);
extern void generate_CALL(quad*);
extern void generate_UMINUS(quad*);
extern void generate_GETRETVAL(quad*);
extern void generate_FUNCSTART(quad*);
extern void generate_RETURN(quad*);
extern void generate_FUNCEND(quad*);

unsigned consts_newstring(char* s);
unsigned consts_newnumber(double n);
unsigned libfuncs_newused(char* s);
unsigned userfuncs_newfunc(SymbolTableEntry* sym);

void make_operand(expr* e, vmarg* arg);
void generate();
void icode_generator();
