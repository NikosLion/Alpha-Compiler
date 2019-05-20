#include "Vm_args.h"

#define AVM_STACKSIZE 4096
#define AVM_TABLE_HASHSIZE 211
#define AVM_WIPEOUT(m) memset(&(m),0,sizeof(m))
#define AVM_STACKENV_SIZE 4
#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v


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


static void avm_initStack();
avm_table* avm_tableNew();
void avm_tableDestroy(avm_table* t);
avm_memcell* avm_tableGetelem(avm_memcell* key);
void avm_tablesetElem(avm_memcell* key,avm_memcell* value);
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