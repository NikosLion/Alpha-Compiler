#include "A_vm.h"

avm_memcell ax,bx,cx;
avm_memcell retval;
unsigned top,topsp;
unsigned totalActuals=0;

unsigned char executionFinished=0;
unsigned      pc=0;
unsigned      currLine=0;
unsigned      codeSize=0;
unsigned      globmem=0;
unsigned      globmem_end=0;

instruction*  code=(instruction*) 0;
userFunc* user_funcs2 = (userFunc*) 0;
strings* lib_funcs2 = (strings*) 0;
strings* string_consts2 = (strings*)0;
double* numbers2 =(double*) 0;

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
  execute_jump,
  execute_nop
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
void avm_initStack(){
  for (unsigned i=0; i<AVM_STACKSIZE; ++i){
    AVM_WIPEOUT(vm_stack[i]);
    vm_stack[i].type=undef_m;
  }
}

///////////////////////////////////////////////////////
void avm_dec_top(){
  //stack overflow
  if(!top){
    avm_error("stack overflow",NULL,NULL,-1);
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
    avm_warning("assigning from 'undef' content!",NULL,NULL);
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

  assert(lv && (&vm_stack[N-1] >= lv && lv>&vm_stack[top] || lv==&retval));
  assert(rv); //8elei ki alla checks

  avm_assign(lv,rv);
}

///////////////////////////////////////////////////////
extern void execute_not(instruction* instr){
  //etoimh apo merikh apotimish
}

///////////////////////////////////////////////////////
extern void execute_jeq(instruction* instr){
  assert(instr->result->type==label_a);
  avm_memcell* rv1 = avm_translate_operand(instr->arg1,&ax);
  avm_memcell* rv2 = avm_translate_operand(instr->arg2,&bx);

  unsigned char result = 0;

  if(rv1->type == undef_m || rv2->type == undef_m){
    avm_error("'undef' involved in equality!",NULL,NULL,-1);
  }
  else if(rv1->type==nil_m || rv2->type==nil_m){
    result=(rv1->type==nil_m && rv2->type==nil_m);
  }
  else if(rv1->type==bool_m || rv2->type==bool_m){
    result=(avm_tobool(rv1)==avm_tobool(rv2));
  }
  else if(rv1->type !=rv2->type){
    avm_error("'%s'=='%s' is illegal!",typeStrings[rv1->type],typeStrings[rv2->type],-1);
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
      avm_error("call:cannot bind '%s' to function!",s,NULL,-1);
      free(s);
      executionFinished=1;
    }
  }
}

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
  int i=0;
  while(instr->result->val != user_funcs2[i].address){
    i++;
  }
  top=top-user_funcs2[i].localSize;
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
  assert(lv && (&vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval));

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

  assert(lv && &vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval);
  assert(t && &vm_stack[N-1]>=t && t>&vm_stack[top]);
  assert(i);

  avm_memcellClear(lv);
  lv->type=nil_m;   /*Default value.*/

  if(t->type!=table_m){
    avm_error("illegal use of type &s as table!",typeStrings[t->type],NULL,-1);
  }
  else{
    avm_memcell* content=avm_tableGetelem(t->data.tableVal, i);
    if(content){
      avm_assign(lv,content);
    }
    else{
      char* ts=avm_tostring(t);
      char* is=avm_tostring(i);
      avm_warning("%s[%s] not found!",ts,is);
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
    avm_error("illegal use of type '%s' as table",typeStrings[t->type],NULL,-1);
  }
  else{
    avm_tablesetElem(t->data.tableVal,i,c);
  }
}

///////////////////////////////////////////////////////
extern void execute_jump(instruction* instr){

}

///////////////////////////////////////////////////////
extern void execute_nop(instruction* instr){
  executionFinished=1;
}

///////////////////////////////////////////////////////
                //MEM_CLEAR
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
unsigned avm_totalactuals(){
  return avm_get_envvalue(topsp+AVM_NUMACTUALS_OFFSET);
}

///////////////////////////////////////////////////////
avm_memcell* avm_getactual(unsigned i){
  assert(i<avm_totalactuals());
  return &vm_stack[topsp+AVM_STACKENV_SIZE+1+i];
}

///////////////////////////////////////////////////////
                //TO BOOLS
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
              //LIBRARY FUNCS
///////////////////////////////////////////////////////

/*With the following every library function is manually
  added in the VM library function resolution map.
*/
void avm_registerlibfunc(char* id,libfunc_t addr){

}

///////////////////////////////////////////////////////
libfunc_t avm_getlibraryfunc(char* id){
  if(strcmp(id,"print")==0){

    (*lib_funcs_table[0])();
  }
  else if(strcmp(id,"typeof")==0){
    (*lib_funcs_table[1])();
  }
  else if(strcmp(id,"input")==0){
    (*lib_funcs_table[2])();
  }
  else if(strcmp(id,"objectmemberkeys")==0){
    (*lib_funcs_table[3])();
  }
  else if(strcmp(id,"objectcopy")==0){
    (*lib_funcs_table[4])();
  }
  else if(strcmp(id,"totalarguments")==0){
    (*lib_funcs_table[5])();
  }
  else if(strcmp(id,"argument")==0){
    (*lib_funcs_table[6])();
  }
  else if(strcmp(id,"strtonum")==0){
    (*lib_funcs_table[7])();
  }
  else if(strcmp(id,"sqrt")==0){
    (*lib_funcs_table[8])();
  }
  else if(strcmp(id,"cos")==0){
    (*lib_funcs_table[10])();
  }
  else if(strcmp(id,"sin")==0){
    (*lib_funcs_table[11])();
  }
}

///////////////////////////////////////////////////////
void avm_initialize(){
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
void avm_calllibfunc(char* id){
  libfunc_t f=avm_getlibraryfunc(id);
  if(!f){
    avm_error("unsupported lib func 's' called!",id,NULL,-1);
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
    retval.data.strVal=strdup(typeStrings[avm_getactual(0)->type]);
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
    avm_error("'totalarguments' called outside a function!",NULL,NULL,-1);
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
  double x=avm_getactual(0)->data.numVal;
  if(x<0){
    fprintf(GOUT,"nil\n");
  }
  else{
    fprintf(GOUT,"%f\n",sqrt(x));
  }
}
///////////////////////////////////////////////////////
void libfunc_cos(){
  fprintf(GOUT,"%f\n",cos(avm_getactual(0)->data.numVal));
}

///////////////////////////////////////////////////////
void libfunc_sin(){
    fprintf(GOUT,"%f\n",sin(avm_getactual(0)->data.numVal));
}

///////////////////////////////////////////////////////
                //TO_STRINGS
///////////////////////////////////////////////////////
char* avm_tostring(avm_memcell* m){
  assert(m->type>=0 && m->type<=undef_m);
  return (*tostringFuncs[m->type])(m);
}

///////////////////////////////////////////////////////
extern char* userfunc_tostring(avm_memcell* m){
  char sfasi[128];
  char* ret;
  sprintf(sfasi,"%d",m->data.funcVal);
  ret=sfasi;
  return ret;
}

///////////////////////////////////////////////////////
extern char* nil_tostring(avm_memcell* m){
  return "null";
}

///////////////////////////////////////////////////////
extern char* undef_tostring(avm_memcell* m){
  return "undefined";
}

///////////////////////////////////////////////////////
extern char* libfunc_tostring(avm_memcell* m){
  return m->data.libfuncVal;
}

///////////////////////////////////////////////////////
extern char* number_tostring(avm_memcell* m){
  char sfasi[128];
  char* ret;
  sprintf(sfasi,"%f",m->data.numVal);
  ret=sfasi;
  return ret;
}

///////////////////////////////////////////////////////
extern char* string_tostring(avm_memcell* m){
  return m->data.strVal;
}

///////////////////////////////////////////////////////
extern char* bool_tostring(avm_memcell* m){
  char sfasi[5];
  char* ret;
  sprintf(sfasi,"%d",m->data.boolVal);
  ret=sfasi;
  return ret;
}

///////////////////////////////////////////////////////
extern char* table_tostring(avm_memcell* m){
    //na to doume
}

///////////////////////////////////////////////////////
                  //ARITHMETIC
///////////////////////////////////////////////////////
void execute_arithmetic(instruction* instr){
  avm_memcell* lv=avm_translate_operand(instr->result,(avm_memcell*) 0);
  avm_memcell* rv1=avm_translate_operand(instr->arg1,&ax);
  avm_memcell* rv2=avm_translate_operand(instr->arg2,&bx);

  assert(lv && (&vm_stack[N-1]>=lv && lv>&vm_stack[top] || lv==&retval));
  assert(rv1 && rv2);

  if(rv1->type != number_a || rv2->type != number_a){
    avm_error("not a number in arithmetic!",NULL,NULL,-1);
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
                //TABLES
///////////////////////////////////////////////////////
avm_table* avm_tableNew(){}

///////////////////////////////////////////////////////
avm_memcell* avm_tableGetelem(avm_table* table,avm_memcell* index){}

///////////////////////////////////////////////////////
void avm_tablesetElem(avm_table* table,avm_memcell* index,avm_memcell* content){}


///////////////////////////////////////////////////////
            //PINAKES CONSTS PHASE 4
///////////////////////////////////////////////////////
double consts_getnumber(unsigned index){
  return numbers2[index];
}

///////////////////////////////////////////////////////
char* const_getstring(unsigned index){
  return string_consts2[index].string;
}

///////////////////////////////////////////////////////
char* libfuncs_getused(unsigned index){
  return lib_funcs2[index].string;
}

///////////////////////////////////////////////////////
              //WARNINGS-ERRORS
///////////////////////////////////////////////////////
extern void avm_warning(char* format,char* c1,char* c2){
  char ch0[256];
  char ch1[256];
  int i=0;
  int j=0;
  int k=0;

  if(c1!=NULL && c2==NULL){
    while(format[i]!='%'){
      ch0[i]=format[i];
      i++;
    }
    strcat(ch0,c1);
    j=j+strlen(ch0);
    i=i+2;
    while(format[i]!='\0'){
      ch0[j]=format[i];
      i++;
      j++;
    }
    fprintf(GOUT,"%s\n",ch0);
  }
  else if(c1!=NULL && c2!=NULL){
    while(format[i]!='%'){
      ch0[i]=format[i];
      i++;
    }
    strcat(ch0,c1);
    j=j+strlen(ch0);
    i=i+2;
    while(format[i]!='%'){
      ch1[k]=format[i];
      i++;
      k++;
    }
    strcat(ch1,c2);
    strcat(ch0,ch1);
    fprintf(GOUT,"%s\n",ch0);
  }
  else{
    fprintf(GOUT,"%s\n",format);
  }
}

///////////////////////////////////////////////////////
extern void avm_error(char* format,char* c1,char* c2,int n){
  char ch0[256];
  char ch1[256];
  int i=0;
  int j=0;
  int k=0;

  if(n==-1){

    if(c1!=NULL && c2==NULL){
      while(format[i]!='%'){
        ch0[i]=format[i];
        i++;
      }
      strcat(ch0,c1);
      j=j+strlen(ch0);
      i=i+2;
      while(format[i]!='\0'){
        ch0[j]=format[i];
        i++;
        j++;
      }
      fprintf(GOUT,"%s\n",ch0);
    }
    else if(c1!=NULL && c2!=NULL){
      while(format[i]!='%'){
        ch0[i]=format[i];
        i++;
      }
      strcat(ch0,c1);
      j=j+strlen(ch0);
      i=i+2;
      while(format[i]!='%'){
        ch1[k]=format[i];
        i++;
        k++;
      }
      strcat(ch1,c2);
      strcat(ch0,ch1);
      fprintf(GOUT,"%s\n",ch0);
    }
    else{
      fprintf(GOUT,"%s\n",format);
    }
  }
  else{
    while(format[i]!='%'){
      ch0[i]=format[i];
      i++;
    }
    fprintf(GOUT,"%s%d",ch0,n);
    i=i+2;
    while(format[i]!='\0'){
      ch1[j]=format[i];
      i++;
      j++;
    }
    fprintf(GOUT,"%s\n",ch1);
  }
}

///////////////////////////////////////////////////////
                //READ BINARY
///////////////////////////////////////////////////////
void Read_froms_Binary(){

  FILE* file2;

  file2=fopen("output","rb");

  if(file2!=NULL){

    struct instruction ins;
    struct userFunc us;
    struct strings lb;
    struct strings st;
    double n;
    int z=0;
    int t=currInstr+curr_nums+curr_funcs+curr_strings+curr_lib_funcs;

    instruction*  temp_code=code;
    userFunc* temp_user_func2=user_funcs2;
    strings* temp_lib_func2=lib_funcs2;
    strings* temp_strings2=string_consts2;
    double* temp_numbers2=numbers2;

    fprintf(GOUT,"\n######################################################################################################################################\n\n");

    while(z<t){
      //instructions talbe
      if(z<currInstr){
        fread(&ins,sizeof(struct instruction),1,file2);
        emit_ins2(&ins);
        temp_code=code+(currCode-1);
        fprintf(GOUT,"Opcode in memory:             %d\n",temp_code->opcode);
      }
      //numbers
      else if(z<currInstr+curr_nums){
        fread(&n,sizeof(double),1,file2);
        consts_newnumber2(n);
        temp_numbers2=numbers2+(curr_nums2-1);
        fprintf(GOUT,"Const numbers in memory:      %f\n",*temp_numbers2);
      }
      //lib_funcs
      else if(z<currInstr+curr_nums+curr_lib_funcs){
        fread(&lb,sizeof(struct strings),1,file2);
        libfuncs_newused2(lb.string);
        temp_lib_func2=lib_funcs2+(curr_lib_funcs2-1);
        fprintf(GOUT,"Lib funcs in memory:          %s\n",temp_lib_func2->string);
      }
      //user_funcs
      else if(z<currInstr+curr_nums+curr_lib_funcs+curr_funcs){
        fread(&us,sizeof(struct userFunc),1,file2);
        userfuncs_newfunc2(&us);
        temp_user_func2=user_funcs2+(curr_funcs2-1);
        fprintf(GOUT,"User funcs address in memory: %d\n",temp_user_func2->address);
      }
      //strings
      else if(z<currInstr+curr_nums+curr_lib_funcs+curr_funcs+curr_strings){
        fread(&st,sizeof(struct strings),1,file2);
        consts_newstring2(st.string);
        temp_strings2=string_consts2+(curr_strings2-1);
        fprintf(GOUT,"Const strings in memory:      %s\n",temp_strings2->string);
      }
      z++;
    }
    fprintf(GOUT,"\n######################################################################################################################################\n\n");
    fclose(file2);
  }
}

///////////////////////////////////////////////////////
void expand3(){
  assert(codeSize == currCode);
  instruction* ins=(instruction*)malloc(NEW_SIZE2);
  if(code){
    memcpy(ins,code,CURR_SIZE2);
    free(code);
  }
  code=ins;
  codeSize=codeSize+EXPAND_SIZE2;
}

///////////////////////////////////////////////////////
void emit_ins2(instruction* instr){
  if(currCode == codeSize){
    expand3();
  }
  instruction* temp;
  temp=code+currCode;
  temp->opcode=instr->opcode;
  temp->arg1=instr->arg1;
  temp->arg2=instr->arg2;
  temp->result=instr->result;
  temp->srcLine=instr->srcLine;
  currCode++;
}

///////////////////////////////////////////////////////
unsigned consts_newstring2(char* s){

  if(curr_strings2 == total_strings2){
    expand_tables2(2);
  }
  strings* new_string;
  new_string=string_consts2+curr_strings2;
  new_string->string=s;
  curr_strings2++;
}

///////////////////////////////////////////////////////
unsigned consts_newnumber2(double n){
  if(curr_nums2 == total_nums2){
    expand_tables2(3);
  }
  double* new_num;
  new_num=numbers2+curr_nums2;
  *new_num=n;
  curr_nums2++;
}

///////////////////////////////////////////////////////
unsigned libfuncs_newused2(char* s){

  if(curr_lib_funcs2 == total_lib_funcs2){
    expand_tables2(1);
  }
  strings* new_lib;
  new_lib=lib_funcs2+curr_lib_funcs2;
  new_lib->string=s;
  curr_lib_funcs2++;
}

///////////////////////////////////////////////////////
unsigned userfuncs_newfunc2(userFunc* sym){

  if(curr_funcs2 == total_funcs2){
    expand_tables2(0);
  }
  struct userFunc *new_func;
  new_func=user_funcs2+curr_funcs2;
  new_func->address=sym->address;
  new_func->id=sym->id;
  new_func->localSize=sym->localSize;
  curr_funcs2++;
}

///////////////////////////////////////////////////////
void expand_tables2(int i){
  if(i==0){
    assert(total_funcs2 == curr_funcs2);
    userFunc* funcs=(userFunc*)malloc(NEW_SIZE2);
    if(user_funcs2){
      memcpy(funcs,user_funcs2,CURR_SIZE2);
      free(user_funcs2);
    }
    user_funcs2=funcs;
    total_funcs2=total_funcs2+EXPAND_SIZE2;
  }
  else if(i==1){
    assert(total_lib_funcs2 == curr_lib_funcs2);
    strings* libs=(strings*)malloc(NEW_SIZE2);
    if(lib_funcs2){
      memcpy(libs,lib_funcs2,CURR_SIZE2);
      free(lib_funcs2);
    }
    lib_funcs2=libs;
    total_lib_funcs2=total_lib_funcs2+EXPAND_SIZE2;
  }
  else if(i==2){
    assert(total_strings2 == curr_strings2);
    strings* strings0=(strings*)malloc(NEW_SIZE2);
    if(string_consts2){
      memcpy(strings0,string_consts2,CURR_SIZE2);
      free(string_consts2);
    }
    string_consts2=strings0;
    total_strings2=total_strings2+EXPAND_SIZE2;
  }
  else if(i==3){
    assert(total_nums2 == curr_nums2);
    double* nums=(double*)malloc(NEW_SIZE2);
    if(numbers2){
      memcpy(nums,numbers2,CURR_SIZE2);
      free(numbers2);
    }
    numbers2=nums;
    total_nums2=total_nums2+EXPAND_SIZE2;
  }
  else{
    fprintf(GOUT,"give right input noob!!!\n");
    assert(0);
  }
}

///////////////////////////////////////////////////////
//init space for global scope space vars to vm_stack
///////////////////////////////////////////////////////
void setGlobmem(){
  struct SymbolTableEntry *temp=getHead();
  struct SymbolTableEntry *temp2=temp;
  int count_globs=0;
  while(temp!=NULL){
    while(temp2!=NULL){
      if((temp2->space == programvar) && (temp2->type!=5)){
        count_globs++;
      }
      temp2=temp2->scope_next;
    }
    temp=temp->scope_list_next;
    temp2=temp;
  }
  globmem=AVM_STACKSIZE;
  globmem_end=AVM_STACKSIZE-count_globs-1;
}
