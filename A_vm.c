#include "A_vm.h"

#define AVM_ENDING_PC codeSize

avm_memcell ax,bx,cx;
avm_memcell retval;
unsigned top,topsp;
unsigned totalActuals=0;

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
tobool_func_t toboolFuncs[]={
  number_tobool,
  string_tobool,
  bool_tobool,
  table_tobool,
  userfunc_tobool,
  libfunc_tobool,
  nil_tobool,
  undef_tobool
};

///////////////////////////////////////////////////////
char* typeStrings[]={
    "number",
    "string",
    "bool",
    "table",
    "userfunc",
    "libfunc"
    "nil",
    "undef"
};

///////////////////////////////////////////////////////
libfunc_t lib_funcs_table[]={
  libfunc_print,
  libfunc_typeof,
  libfunc_input,
  libfunc_objectmemberkeys,
  libfunc_objectcopy,
  libfunc_totalarguments,
  libfunc_argument,
  libfunc_strtonum,
  libfunc_sqrt,
  libfunc_cos,
  libfunc_sin
};

tostring_func_t tostringFuncs[]={
  number_tostring,
  string_tostring,
  bool_tostring,
  table_tostring,
  userfunc_tostring,
  libfunc_tostring,
  nil_tostring,
  undef_tostring
};

arithmetic_func_t arithmeticFuncs[]={
  add_impl,
  sub_impl,
  mul_impl,
  div_impl,
  mod_impl
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
extern void avm_warning(char* format){

}

///////////////////////////////////////////////////////
extern void avm_error(char* format,char* name,char* name2,unsigned n){

}

///////////////////////////////////////////////////////
void avm_calllibfunc(char* id){
  library_func_t f=avm_getlibraryfunc(id);
  if(!f){
    avm_error("unsupported lib func 's' called!",id,NULL,0);
    executionFinished=1;
  }
  else{
    topsp=top;
    totalActuals=0;
    (*f)();
    if(!executionFinished){
      execute_funcexit((instruction*)0);
    }
  }
}

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
extern void execute_assign(instruction* instr){
  avm_memcell* lv=avm_translate_operand(instr->result,(avm_memcell*) 0);
  avm_memcell* rv=avm_translate_operand(instr->arg1,&ax);

//  assert(lv && (&vm_stack[N-1] >= lv && lv>&vm_stack[top] || lv==&retval));
  //assert(rv);//should do similar assertion tests here

  avm_assign(lv,rv);
}

///////////////////////////////////////////////////////
extern void execute_not(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_jeq(instruction* instr){
  assert(instr->result->type==label_a);
  avm_memcell* rv1 = avm_translate_operand(instr->arg1,&ax);
  avm_memcell* rv2 = avm_translate_operand(instr->arg2,&bx);

  unsigned char result = 0;

  if(rv1->type == undef_m || rv2->type == undef_m){
    avm_error("'undef' involved in equality!",NULL,NULL,0);
  }
  else if(rv1->type==nil_m || rv2->type==nil_m){
    result=(rv1->type==nil_m && rv2->type==nil_m);
  }
  else if(rv1->type==bool_m || rv2->type==bool_m){
    result=(avm_tobool(rv1)==avm_tobool(rv2));
  }
  else if(rv1->type !=rv2->type){
    avm_error("'%s'=='%s' is illegal!",typeStrings[rv1->type],typeStrings[rv2->type],0);
  }
  else{

  }
  if(!executionFinished && result){
    pc=instr->result->val;
  }
}

///////////////////////////////////////////////////////
extern void execute_jne(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_jle(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_jge(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_jlt(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_jgt(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_call(instruction* instr){
  avm_memcell*func=avm_translate_operand(instr->result,&ax);
  assert(func);
  avm_callsaveenvionment();

  switch (func->type) {
    case userfunc_m :{
      pc=func->data.funcVal;
      assert(pc<AVM_ENDING_PC);
      assert(code[pc].opcode==funcenter_v);
      break;
    }
    case string_m :{
      avm_calllibfunc(func->data.strVal);
      break;
    }
    case libfunc_m :{
      avm_calllibfunc(func->data.libfuncVal);
      break;
    }
    default: {
      char *s=avm_tostring(func);
      avm_error("call:cannot bind '%s' to function!",s,NULL,0);
      free(s);
      executionFinished=1;
    }
  }
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
extern void execute_pusharg(instruction* instr){
  avm_memcell* arg=avm_translate_operand(instr->arg1,&ax);
  assert(arg);

  /*This is actually stack[top]=arg, but we have to
    use avm_assign.
  */
  avm_assign(&vm_stack[top],arg);
  ++totalActuals;
  avm_dec_top();
}

///////////////////////////////////////////////////////
extern void execute_funcenter(instruction* instr){
  avm_memcell* func =avm_translate_operand(instr->result,&ax);
  assert(func);
  assert(pc=func->data.funcVal);

  //calle actions here
  totalActuals=0;
  topsp=top;
//  top=top-funcInfo->localSize;
}

///////////////////////////////////////////////////////
extern void execute_funcexit(instruction* instr){
  unsigned oldTop=top;
  top=avm_get_envvalue(topsp+AVM_SAVEDTOP_OFFSET);
  pc=avm_get_envvalue(topsp+AVM_SAVEDPC_OFFSET);
  topsp=avm_get_envvalue(topsp+AVM_SAVEDTOPSP_OFFSET);

  while(++oldTop<=top){
    avm_memcellClear(&vm_stack[oldTop]);
  }
}

///////////////////////////////////////////////////////
extern void execute_newtable(instruction* instr){
  avm_memcell* lv=avm_translate_operand(instr->result, (avm_memcell*) 0);
  //assert(lv && (&vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval));

  avm_memcellClear(lv);

  lv->type=table_m;
  lv->data.tableVal=avm_tableNew();
  avm_tableincrefcounter(lv->data.tableVal);
}

///////////////////////////////////////////////////////
extern void execute_tablegetelem(instruction* instr){
  avm_memcell* lv=avm_translate_operand(instr->result, (avm_memcell*) 0);
  avm_memcell* t=avm_translate_operand(instr->arg1, (avm_memcell*) 0);
  avm_memcell* i=avm_translate_operand(instr->arg2, &ax);

  //assert(lv && &vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval);
  //assert(t && &vm_stack[N-1]>=t && t>&vm_stack[top]);
  assert(i);

  avm_memcellClear(lv);
  lv->type=nil_m;   /*Default value.*/

  if(t->type!=table_m){
    //avm_error("illegal use of type &s as table!", typeStrings[t->type]);
  }
  else{
    avm_memcell* content=avm_tableGetelem(t->data.tableVal, i);
    if(content){
      avm_assign(lv,content);
    }
    else{
      char* ts=avm_tostring(t);
      char* is=avm_tostring(i);
      //avm_warning("%s[%s] not found!",ts,is);
      free(ts);
      free(is);
    }
  }
}

///////////////////////////////////////////////////////
extern void execute_tablesetelem(instruction* instr){
  avm_memcell* t=avm_translate_operand(instr->result,(avm_memcell*)0);
  avm_memcell* i=avm_translate_operand(instr->arg1,&ax);
  avm_memcell* c=avm_translate_operand(instr->arg2,&bx);

  assert(t && &vm_stack[N-1]>=t && t>&vm_stack[top]);
  assert(i && c);

  if(t->type != table_m){
    avm_error("illegal use of type '%s' as table",typeStrings[t->type],NULL,0);
  }
  else{
    avm_tablesetElem(t->data.tableVal,i,c);
  }
}

///////////////////////////////////////////////////////
extern void execute_nop(instruction* instr){

}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
void avm_dec_top(){
  //stack overflow
  if(!top){
    avm_error("stack overflow",NULL,NULL,0);
    executionFinished=1;
  }
  else{
    --top;
  }
}

///////////////////////////////////////////////////////
void avm_push_envvalue(unsigned val){
  vm_stack[top].type=number_m;
  vm_stack[top].data.numVal=val;
  avm_dec_top();
}

///////////////////////////////////////////////////////
void avm_callsaveenvionment(){
  avm_push_envvalue(totalActuals);
  avm_push_envvalue(pc+1);
  avm_push_envvalue(top+totalActuals+2);
  avm_push_envvalue(topsp);
}

///////////////////////////////////////////////////////
unsigned avm_get_envvalue(unsigned i){
  assert(vm_stack[i].type=number_m);
  unsigned val=(unsigned)vm_stack[i].data.numVal;
  assert(vm_stack[i].data.numVal==((double)val));
  return val;
}

///////////////////////////////////////////////////////
library_func_t avm_getlibraryfunc(char* id){

}

///////////////////////////////////////////////////////
unsigned char number_tobool(avm_memcell* m){
  return m->data.numVal != 0;
}

///////////////////////////////////////////////////////
unsigned char string_tobool(avm_memcell* m){
    return m->data.strVal[0] != 0;
}

///////////////////////////////////////////////////////
unsigned char bool_tobool(avm_memcell* m){
    return m->data.boolVal;
}

///////////////////////////////////////////////////////
unsigned char table_tobool(avm_memcell* m){
    return 1;
}

///////////////////////////////////////////////////////
unsigned char userfunc_tobool(avm_memcell* m){
  return 1;
}

///////////////////////////////////////////////////////
unsigned char libfunc_tobool(avm_memcell* m){
  return 1;
}

///////////////////////////////////////////////////////
unsigned char nil_tobool(avm_memcell* m){
  return 0;
}

///////////////////////////////////////////////////////
unsigned char undef_tobool(avm_memcell* m){
  assert(0);
  return 0;
}

///////////////////////////////////////////////////////
unsigned char avm_tobool(avm_memcell* m){
  assert(m->type>=0 && m->type<undef_m);
  return (*toboolFuncs[m->type])(m);
}

///////////////////////////////////////////////////////


///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
unsigned avm_totalactuals(){
  return avm_get_envvalue(topsp+AVM_NUMACTUALS_OFFSET);
}

///////////////////////////////////////////////////////
avm_memcell* avm_getactual(unsigned i){
  assert(i<avm_totalactuals());
  return &vm_stack[topsp+AVM_STACKENV_SIZE+1+i];
}

///////////////////////////////////////////////////////
/*With the following every library function is manually
  added in the VM library function resolution map.
*/
void avm_registerlibfunc(char* id,library_func_t addr){

}

///////////////////////////////////////////////////////
char* avm_tostring(avm_memcell* m){
  assert(m->type>=0 && m->type<=undef_m);
  return (*tostringFuncs[m->type])(m);
}

///////////////////////////////////////////////////////
extern char* number_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* string_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* bool_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* table_tostring(avm_memcell* m){

}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
void avm_initialize(){
  avm_initStack();
  avm_registerlibfunc("print",libfunc_print);
  avm_registerlibfunc("typeof",libfunc_typeof);
  avm_registerlibfunc("input",libfunc_input);
  avm_registerlibfunc("objectmemberkeys",libfunc_objectmemberkeys);
  avm_registerlibfunc("objectcopy",libfunc_objectcopy);
  avm_registerlibfunc("totalarguments",libfunc_totalarguments);
  avm_registerlibfunc("argument",libfunc_argument);
  avm_registerlibfunc("strtonum",libfunc_strtonum);
  avm_registerlibfunc("sqrt",libfunc_sqrt);
  avm_registerlibfunc("cos",libfunc_cos);
  avm_registerlibfunc("sin",libfunc_sin);
}

///////////////////////////////////////////////////////
/*Implementation of the library function 'print'.
  It displays every argument at the console.
*/
void libfunc_print(){
  unsigned n=avm_totalactuals();
  for(unsigned i=0;i<n;++i){
    char* s=avm_tostring(avm_getactual(i));
    puts(s);
    free(s);
  }
}

///////////////////////////////////////////////////////
void libfunc_typeof(){
  unsigned n =avm_totalactuals();
  if(n!=1){
    avm_error("one argument (not %d) expected in 'typeof'! ",NULL,NULL,n);
  }
  else{
    avm_memcellClear(&retval); //dont forget to clean it up
    retval.type=string_m;
    //retval.data.strVal=strdup(typeStrings[avm_getactual()]);
  }
}

///////////////////////////////////////////////////////
void libfunc_input(){

}

///////////////////////////////////////////////////////
void libfunc_objectmemberkeys(){

}

///////////////////////////////////////////////////////
void libfunc_objectcopy(){

}

///////////////////////////////////////////////////////
void libfunc_totalarguments(){
  unsigned p_topsp=avm_get_envvalue(topsp+AVM_SAVEDTOPSP_OFFSET);
  avm_memcellClear(&retval);

  if(!p_topsp){
    avm_error("'totalarguments' called outside a function!",NULL,NULL,0);
    retval.type=nil_m;
  }
  else{
    retval.type=number_m;
    retval.data.numVal=avm_get_envvalue(p_topsp+AVM_NUMACTUALS_OFFSET);
  }
}

///////////////////////////////////////////////////////
void libfunc_argument(){

}

///////////////////////////////////////////////////////
void libfunc_strtonum(){

}

///////////////////////////////////////////////////////
void libfunc_sqrt(){

}
///////////////////////////////////////////////////////
void libfunc_cos(){

}

///////////////////////////////////////////////////////
void libfunc_sin(){

}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
extern char* userfunc_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* nil_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* undef_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
extern char* libfunc_tostring(avm_memcell* m){

}

///////////////////////////////////////////////////////
double add_impl(double x,double y){return x+y;}

///////////////////////////////////////////////////////
double sub_impl(double x,double y){return x-y;}

///////////////////////////////////////////////////////
double mul_impl(double x,double y){return x*y;}

///////////////////////////////////////////////////////
double div_impl(double x,double y){
  if(y!=0){
    return x/y;
  }
}

///////////////////////////////////////////////////////
double mod_impl(double x,double y){
  if(y!=0){
    return ((unsigned) x) % ((unsigned) y);
  }
}

///////////////////////////////////////////////////////
void execute_arithmetic(instruction* instr){
  avm_memcell* lv=avm_translate_operand(instr->result,(avm_memcell*) 0);
  avm_memcell* rv1=avm_translate_operand(instr->arg1,&ax);
  avm_memcell* rv2=avm_translate_operand(instr->arg2,&bx);

  //assert(lv && (&vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval));
  assert(rv1 && rv2);

  if(rv1->type != number_a || rv2->type != number_a){
    //avm_error("not a number in arithmetic!"); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    executionFinished=1;
  }
  else{
    arithmetic_func_t op=arithmeticFuncs[instr->opcode-add_v];
    avm_memcellClear(lv);
    lv->type=number_m;
    lv->data.numVal=(*op)(rv1->data.numVal,rv2->data.numVal);
  }
}

///////////////////////////////////////////////////////
avm_table* avm_tableNew(){


}

///////////////////////////////////////////////////////
avm_memcell* avm_tableGetelem(avm_table* table,avm_memcell* index){


}

///////////////////////////////////////////////////////
void avm_tablesetElem(avm_table* table,avm_memcell* index,avm_memcell* content){


}
