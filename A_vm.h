#include "Vm_args.h"

#define AVM_STACKSIZE 4096
#define N 4096
#define AVM_TABLE_HASHSIZE 211
#define AVM_WIPEOUT(m) memset(&(m),0,sizeof(m))
#define AVM_STACKENV_SIZE 4
#define AVM_ENDING_PC codeSize
#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v
#define AVM_NUMACTUALS_OFFSET 4
#define AVM_SAVEDPC_OFFSET 3
#define AVM_SAVEDTOP_OFFSET 2
#define AVM_SAVEDTOPSP_OFFSET 1

///////////////////////////////////////////////////////
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

unsigned total_funcs2;
unsigned total_lib_funcs2;
unsigned total_strings2;
unsigned total_nums2;
unsigned curr_funcs2;
unsigned curr_lib_funcs2;
unsigned curr_strings2;
unsigned curr_nums2;


enum avm_memcell_t{
  number_m=0,
  string_m=1,
  bool_m=2,
  table_m=3,
  userfunc_m=4,
  libfunc_m=5,
  nil_m=6,
  undef_m=7
};

typedef struct avm_memcell{
  enum avm_memcell_t type;
  union{
    double numVal;
    char* strVal;
    unsigned char boolVal;
    struct avm_table* tableVal;
    unsigned funcVal;
    char* libfuncVal;
  }data;
}avm_memcell;

typedef struct avm_table_bucket{
  struct  avm_memcell key;
  struct  avm_memcell value;
  struct  avm_table_bucket* next;
}avm_table_bucket;

typedef struct avm_table{
  unsigned refCounter;
  struct avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
  struct avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
  unsigned total;
}avm_table;

avm_memcell vm_stack[AVM_STACKSIZE];
typedef void (*memclear_func_t)(avm_memcell*);
typedef void (*execute_func_t)(instruction*);
typedef void (*libfunc_t)();
libfunc_t avm_getlibraryfunc(char* id); //typical hashing



void avm_initStack();
avm_table* avm_tableNew();
void avm_tableDestroy(avm_table* t);

avm_memcell* avm_tableGetelem(avm_table* table,avm_memcell* index);
void avm_tablesetElem(avm_table* table,avm_memcell* index,avm_memcell* content);
void avm_memcellClear(avm_memcell* m);
void avm_tableBucketsDestroy(avm_table_bucket** p);
void avm_tablebucketsInit(avm_table_bucket** p);
void avm_tableincrefcounter(avm_table* t);
void avm_tabledecrefcounter(avm_table* t);
double consts_getnumber(unsigned index);
char* const_getstring(unsigned index);
char* libfuncs_getused(unsigned index);
avm_memcell* avm_translate_operand(vmarg* arg,avm_memcell*reg);
extern void memclear_string(avm_memcell* m);
extern void memclear_table(avm_memcell* m);
void execute_cycle();

extern void avm_warning(char*,char*,char*);
extern void avm_error(char* format,char* name,char* name2,unsigned n);
extern void avm_calllibfunc(char* funcName);
extern void avm_assign(avm_memcell* lv,avm_memcell* rv);

///////////////////////////////////////////////////////
extern void execute_assign(instruction*);
extern void execute_add(instruction*);
extern void execute_sub(instruction*);
extern void execute_mul(instruction*);
extern void execute_div(instruction*);
extern void execute_mod(instruction*);
extern void execute_not(instruction*);
extern void execute_jeq(instruction*);
extern void execute_jne(instruction*);
extern void execute_jle(instruction*);
extern void execute_jge(instruction*);
extern void execute_jlt(instruction*);
extern void execute_jgt(instruction*);
extern void execute_call(instruction*);
extern void execute_pusharg(instruction*);
extern void execute_funcenter(instruction*);
extern void execute_funcexit(instruction*);
extern void execute_newtable(instruction*);
extern void execute_tablegetelem(instruction*);
extern void execute_tablesetelem(instruction*);
extern void execute_nop(instruction*);
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
typedef char* (*tostring_func_t)(avm_memcell*);

extern char* avm_tostring(avm_memcell*);
extern char* number_tostring(avm_memcell*);
extern char* string_tostring(avm_memcell*);
extern char* bool_tostring(avm_memcell*);
extern char* table_tostring(avm_memcell*);
extern char* userfunc_tostring(avm_memcell*);
extern char* libfunc_tostring(avm_memcell*);
extern char* nil_tostring(avm_memcell*);
extern char* undef_tostring(avm_memcell*);

///////////////////////////////////////////////////////
typedef double (*arithmetic_func_t)(double x,double y);

double add_impl(double x,double y);
double sub_impl(double x,double y);
double mul_impl(double x,double y);
double div_impl(double x,double y);
double mod_impl(double x,double y);
void execute_arithmetic(instruction* instr);
///////////////////////////////////////////////////////


extern void avm_callsaveenvionment();
void avm_dec_top();
void avm_push_envvalue(unsigned val);
extern userFunc* avm_getfuncinfo(unsigned address);
unsigned avm_get_envvalue(unsigned i);


///////////////////////////////////////////////////////
typedef unsigned char (*tobool_func_t)(avm_memcell*);
unsigned char number_tobool(avm_memcell* m);
unsigned char string_tobool(avm_memcell* m);
unsigned char bool_tobool(avm_memcell* m);
unsigned char table_tobool(avm_memcell* m);
unsigned char userfunc_tobool(avm_memcell* m);
unsigned char libfunc_tobool(avm_memcell* m);
unsigned char nil_tobool(avm_memcell* m);
unsigned char undef_tobool(avm_memcell* m);
unsigned char avm_tobool(avm_memcell* m);
//////////////////////////////////////////////////////
extern void libfunc_print();
extern void libfunc_typeof();
extern void libfunc_input();
extern void libfunc_objectmemberkeys();
extern void libfunc_objectcopy();
extern void libfunc_totalarguments();
extern void libfunc_argument();
extern void libfunc_strtonum();
extern void libfunc_sqrt();
extern void libfunc_cos();
extern void libfunc_sin();
//////////////////////////////////////////////////////
void avm_initialize();
void libfunc_totalarguments();
void avm_registerlibfunc(char* c,libfunc_t u);
avm_memcell* avm_getactual(unsigned i);
unsigned avm_totalactuals();
void Read_froms_Binary();
void emit_ins2(instruction* instr);
unsigned consts_newstring2(char* s);
unsigned consts_newnumber2(double n);
unsigned libfuncs_newused2(char* s);
unsigned userfuncs_newfunc2(userFunc* sym);
void expand_tables2(int i);
void setGlobmem();
