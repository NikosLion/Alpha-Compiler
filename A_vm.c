#include "A_vm.h"

#define AVM_ENDING_PC codeSize

avm_memcell ax,bx,cx;
avm_memcell retval;
unsigned top,topsp;

unsigned char executionFinished=0;
unsigned      pc=0;
unsigned      currLine=0;
unsigned      codeSize=0;
instruction*  code=(instruction*) 0;

memclear_func_t memclearFuncs[]={
  0, //number
  memclear_string,
  0, //bool
  memclear_table,
  0, //userfunc
  0, //libfunc
  0, //nil
  0 //undef
};

///////////////////////////////////////////////////////
execute_func_t excecuteFuncs[]={
  execute_assign,
  execute_add,
  execute_sub,
  execute_mul,
  execute_div,
  execute_mod,
  execute_not,
  execute_jeq,
  execute_jne,
  execute_jle,
  execute_jge,
  execute_jlt,
  execute_jgt,
  execute_call,
  execute_pusharg,
  execute_funcenter,
  execute_funcexit,
  execute_newtable,
  execute_tablegetelem,
  execute_tablesetelem,
  execute_nop
};


///////////////////////////////////////////////////////
static void avm_initStack(){
  for (unsigned i=0; i<AVM_STACKSIZE; ++i){
    AVM_WIPEOUT(vm_stack[i]);
    vm_stack[i].type=undef_m;
  }
}

///////////////////////////////////////////////////////
void avm_tableDestroy(avm_table* t){
  avm_tableBucketsDestroy(t->strIndexed);
  avm_tableBucketsDestroy(t->numIndexed);
  free(t);
}

///////////////////////////////////////////////////////
void avm_tableBucketsDestroy(avm_table_bucket** p){
  for(unsigned i=0; i<AVM_TABLE_HASHSIZE; ++i, ++p){
    for(avm_table_bucket* b=*p; b;){
      avm_table_bucket* del=b;
      b=b->next;
      avm_memcellClear(&del->key);
      avm_memcellClear(&del->value);
      free(del);
    }
    p[i]=(avm_table_bucket*) 0;
  }
}

///////////////////////////////////////////////////////
void avm_tableincrefcounter(avm_table* t){
  ++t->refCounter;
}

///////////////////////////////////////////////////////
void avm_tabledecrefcounter(avm_table* t){
  assert(t->refCounter>0);
  if(!--t->refCounter){
    avm_tableDestroy(t);
  }
}

///////////////////////////////////////////////////////
void avm_tablebucketsInit(avm_table_bucket** p){
  for(unsigned i=0; i<AVM_TABLE_HASHSIZE; ++i){
    p[i]=(avm_table_bucket*)0;
  }
}

///////////////////////////////////////////////////////
void avm_memcellClear(avm_memcell* m){
  if(m->type!=undef_m){
    memclear_func_t f=memclearFuncs[m->type];
    if(f){
      (*f)(m);
    }
    m->type=undef_m;
  }
}

///////////////////////////////////////////////////////
double consts_getnumber(unsigned index){

}

///////////////////////////////////////////////////////
char* const_getstring(unsigned index){

}

///////////////////////////////////////////////////////
char* libfuncs_getused(unsigned index){

}

///////////////////////////////////////////////////////
avm_memcell* avm_translate_operand(vmarg* arg,avm_memcell* reg){
  switch (arg->type) {
    case global_a:  return &vm_stack[AVM_STACKSIZE-1-arg->val];
    case local_a:   return &vm_stack[topsp-arg->val];
    case formal_a:  return &vm_stack[topsp+AVM_STACKENV_SIZE+1+arg->val];
    case retval_a:  return &retval;
    case number_a:  {
      reg->type=number_m;
      reg->data.numVal=consts_getnumber(arg->val);
      return reg;
    }
    case string_a:  {
      reg->type=string_m;
      reg->data.strVal=strdup(const_getstring(arg->val));
      return reg;
    }
    case bool_a:  {
      reg->type=bool_m;
      reg->data.boolVal=arg->val;
      return reg;
    }
    case nil_a: {
      reg->type=nil_m;
      return reg;
    }
    case userfunc_a:  {
      reg->type=userfunc_m;
      reg->data.funcVal=arg->val;
      return reg;
    }
    case libfunc_a: {
      reg->type=libfunc_m;
      reg->data.libfuncVal=libfuncs_getused(arg->val);
      return reg;
    }
    default: assert(0);
  }
}

///////////////////////////////////////////////////////
void execute_cycle(){
  if(executionFinished){
    return;
  }
  else if(pc==AVM_ENDING_PC){
    executionFinished=1;
    return;
  }
  else{
    assert(pc<AVM_ENDING_PC);
    instruction* instr=code+pc;
    assert(instr->opcode>=0 && instr->opcode<=AVM_MAX_INSTRUCTIONS);
    if(instr->srcLine){
      currLine=instr->srcLine;
    }
    unsigned oldPC=pc;
    (*excecuteFuncs[instr->opcode])(instr);
    if(pc==oldPC){
      ++pc;
    }
  }
}

///////////////////////////////////////////////////////
extern void memclear_string(avm_memcell* m){
  assert(m->data.strVal);
  free(m->data.strVal);
}

///////////////////////////////////////////////////////
extern void memclear_table(avm_memcell* m){
  assert(m->data.tableVal);
  avm_tabledecrefcounter(m->data.tableVal);
}

///////////////////////////////////////////////////////
//extern void avm_warning(char* format,...){}

///////////////////////////////////////////////////////
extern void avm_assign(avm_memcell* lv,avm_memcell* rv){
  /*Same cells? destructive to assign!*/
  if(lv==rv){
    return;
  }
  /*Same tables?No need to assign!*/
  if(lv->type==table_m && rv->type==table_m && lv->data.tableVal==rv->data.tableVal){
    return;
  }
  /*From undefined r-value?warning*/
  if(rv->type==undef_m){
    avm_warning("assigning from 'undef' content!");
  }

  /*Clear old cell contents.*/
  avm_memcellClear(lv);
  /*In C++ dispatch instead.*/
  memcpy(lv,rv,sizeof(avm_memcell));

  /*Now take care of copied values or reference counting*/
  if(lv->type==string_m){
    lv->data.strVal=strdup(rv->data.strVal);
  }
  else if(lv->type==table_m){
    avm_tableincrefcounter(lv->data.tableVal);
  }
}

///////////////////////////////////////////////////////
extern void execute_assign(instruction* temp){
  avm_memcell* lv=avm_translate_operand(&temp->result,(avm_memcell*) 0);
  avm_memcell* rv=avm_translate_operand(&temp->arg1,&ax);

  assert(lv && (&vm_stack[N-1] >= lv && lv>&vm_stack[top] || lv==&retval));
  assert(rv)//should do similar assertion tests here

  avm_assign(lv,rv);
}

///////////////////////////////////////////////////////
extern void execute_add(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_sub(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_mul(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_div(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_mod(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_not(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jeq(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jne(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jle(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jge(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jlt(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_jgt(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_call(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_pusharg(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_funcenter(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_funcexit(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_newtable(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_tablegetelem(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_tablesetelem(instruction* temp){

}

///////////////////////////////////////////////////////
extern void execute_nop(instruction* temp){

}
