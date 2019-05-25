#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Table_queue.h"


#define EXPAND_SIZE2 1024
#define CURR_SIZE2 (total*sizeof(instruction))
#define NEW_SIZE2 (EXPAND_SIZE2*sizeof(instruction)+CURR_SIZE2)

unsigned total2;
unsigned total_funcs;
unsigned total_lib_funcs;
unsigned total_strings;
unsigned total_nums;
unsigned curr_funcs;
unsigned curr_lib_funcs;
unsigned curr_strings;
unsigned curr_nums;
int currInstr;
struct incomplete_jump* ij_head;
struct func_stack_entry* f_stack;

enum vmopcode{
  assign_v,       add_v,          sub_v,
  mul_v,          div_v,          mod_v,
  not_v,          jeq_v,          jne_v,
  jle_v,          jge_v,          jlt_v,
  jgt_v,          call_v,         pusharg_v,
  funcenter_v,    funcexit_v,     newtable_v,
  tablegetelem_v, tablesetelem_v, jump_v,
  nop_v
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
  vmarg*     result;
  vmarg*     arg1;
  vmarg*     arg2;
  unsigned  srcLine;
}instruction;

typedef struct userFunc{
  unsigned  address;
  unsigned  localSize;
  char*     id;
}userFunc;

typedef struct strings{
  char* string;
}strings;

typedef struct incomplete_jump{
  unsigned instrNo; //the jump instruction number
  unsigned iaddress;  //the i-code jump target address
  struct incomplete_jump* next;
}incomplete_jump;

typedef struct func_stack_entry{
  SymbolTableEntry* function;
  struct func_stack_entry* next;
}func_stack_entry;

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
void expand2();
void make_operand(expr* e, vmarg* arg);
void call_generators();
void generate(enum vmopcode  opcode,quad* q);
void icode_generator();
void emit_ins(instruction* t);
void print_instructions_table();
void generate_relational (enum vmopcode opcode, quad* q);
void add_incomplete_jump(unsigned instrNo,unsigned iaddress);
void patch_incomplete_jumps();
char* return_instraction_op(int op);
void make_numberoperand(vmarg* arg,double val);
void make_booloperand(vmarg* arg,unsigned val);
void make_retvaloperand(vmarg* arg);
void insert_ret_list(unsigned l,SymbolTableEntry* f);
void backpatch_ret_list(unsigned l,SymbolTableEntry* f);
void push_func(struct SymbolTableEntry* f);
SymbolTableEntry* pop_func();
SymbolTableEntry* top_func();
void expand_tables(int i);
void nop_emit();
FILE* convert_to_binary();
