#include "Vm_args.h"
#include <stdlib.h>



int quad_runner;

generator_func_t generators[]= {
  generate_ADD,
  generate_SUB,
  generate_MUL,
  generate_DIV,
  generate_MOD,
  generate_NEWTABLE,
  generate_TABLEGETELEM,
  generate_TABLESETELEM,
  generate_ASSIGN,
  generate_JUMP,
  generate_IF_EQ,
  generate_IF_NOTEQ,
  generate_IF_GREATER,
  generate_IF_GREATEREQ,
  generate_IF_LESS,
  generate_IF_LESSEQ,
  generate_NOT,
  generate_PARAM,
  generate_CALL,
  generate_UMINUS,
  generate_GETRETVAL,
  generate_FUNCSTART,
  generate_RETURN,
  generate_FUNCEND
};

instruction* instructions = (instruction*) 0;
userFunc* user_funcs = (userFunc*) 0;
strings* lib_funcs = (strings*) 0;
strings* string_consts = (strings*)0;
double* numbers =(double*) 0;


///////////////////////////////////////////////////////
unsigned consts_newstring(char* s){

  if(curr_strings == total_strings){
    expand_tables(2);
  }
  strings* new_string;
  new_string=string_consts+curr_strings;
  new_string->string=s;
  curr_strings++;
}

///////////////////////////////////////////////////////
unsigned consts_newnumber(double n){
  if(curr_nums == total_nums){
    expand_tables(3);
  }
  double* new_num;
  new_num=numbers+curr_nums;
  *new_num=n;
  curr_nums++;
}

///////////////////////////////////////////////////////
unsigned libfuncs_newused(char* s){

  if(curr_lib_funcs == total_lib_funcs){
    expand_tables(1);
  }
  strings* new_lib;
  new_lib=lib_funcs+curr_lib_funcs;
  new_lib->string=s;
  curr_lib_funcs++;
}

///////////////////////////////////////////////////////
unsigned userfuncs_newfunc(SymbolTableEntry* sym){

  if(curr_funcs == total_funcs){
    expand_tables(0);
  }
  struct userFunc *new_func;
  new_func=user_funcs+curr_funcs;
  new_func->address=sym->taddress;
  new_func->id=sym->name;
  new_func->localSize=sym->func_locals;
  curr_funcs++;
}

///////////////////////////////////////////////////////
void expand_tables(int i){
  if(i==0){
    assert(total_funcs == curr_funcs);
    userFunc* funcs=(userFunc*)malloc(NEW_SIZE2);
    if(user_funcs){
      memcpy(funcs,user_funcs,CURR_SIZE2);
      free(user_funcs);
    }
    user_funcs=funcs;
    total_funcs=total_funcs+EXPAND_SIZE2;
  }
  else if(i==1){
    assert(total_lib_funcs == curr_lib_funcs);
    strings* libs=(strings*)malloc(NEW_SIZE2);
    if(lib_funcs){
      memcpy(libs,lib_funcs,CURR_SIZE2);
      free(lib_funcs);
    }
    lib_funcs=libs;
    total_lib_funcs=total_lib_funcs+EXPAND_SIZE2;
  }
  else if(i==2){
    assert(total_strings == curr_strings);
    strings* strings0=(strings*)malloc(NEW_SIZE2);
    if(string_consts){
      memcpy(strings0,string_consts,CURR_SIZE2);
      free(string_consts);
    }
    string_consts=strings0;
    total_strings=total_strings+EXPAND_SIZE2;
  }
  else if(i==3){
    assert(total_nums == curr_nums);
    double* nums=(double*)malloc(NEW_SIZE2);
    if(numbers){
      memcpy(nums,numbers,CURR_SIZE2);
      free(numbers);
    }
    numbers=nums;
    total_nums=total_nums+EXPAND_SIZE2;
  }
  else{
    fprintf(GOUT,"give right input noob!!!\n");
    assert(0);
  }
}

///////////////////////////////////////////////////////
void expand2(){
  assert(total2 == currInstr);
  instruction* ins=(instruction*)malloc(NEW_SIZE2);
  if(instructions){
    memcpy(ins,instructions,CURR_SIZE2);
    free(instructions);
  }
  instructions=ins;
  total2=total2+EXPAND_SIZE2;
}

///////////////////////////////////////////////////////
void emit_ins(instruction* instr){
  if(currInstr == total2){
    expand2();
  }
  instruction* temp;
  temp=instructions+currInstr;
  temp->opcode=instr->opcode;
  temp->arg1=instr->arg1;
  temp->arg2=instr->arg2;
  temp->result=instr->result;
  temp->srcLine=instr->srcLine;
  currInstr++;
}

///////////////////////////////////////////////////////
void call_generators (){
  for(quad_runner=0;quad_runner<currQuad;++quad_runner){
    (*generators[(getQuads()+quad_runner)->op])(getQuads()+quad_runner);
  }
  nop_emit();
}

///////////////////////////////////////////////////////
void make_operand(expr* e, vmarg* arg){
  switch(e->type){
    case var_e:
    case tableitem_e:
    case arithexpr_e:
    case boolexpr_e:
    case newtable_e:{
      if(e->sym==NULL){
        exit(0);
      }
      arg->val=e->sym->offset;

      switch(e->sym->space){
        case programvar:  arg->type=global_a;break;
        case functionlocal: arg->type=local_a; break;
        case formalarg :   arg->type=formal_a; break;
        default: exit(0);
      }
      break; //from case newtable_e
    }
    case constbool_e:{
      arg->val=e->value.boolean;
      arg->type=bool_a;
      break;
    }
    case conststring_e:{
      arg->val=consts_newstring(e->value.stringValue);
      arg->type=string_a;
      break;
    }
    case constnum_e:{
      if(e->int_real==0){
        arg->val=consts_newnumber(e->value.realValue);
      }
      else if(e->int_real==1){
        arg->val=consts_newnumber(e->value.intValue);
      }
      arg->type=number_a;
      break;
    }
    case nil_e:{
      arg->type=nil_a;
      break;
    }
    case programfunc_e:{
      arg->type=userfunc_a;
      arg->val=currInstr;
      break;
    }
    case libraryfunc_e:{
      arg->type=libfunc_a;
      arg->val=libfuncs_newused(e->sym->name);
      break;
    }
    default:exit(0);
  }
}

///////////////////////////////////////////////////////
void make_numberoperand(vmarg* arg,double val){
  arg->val=consts_newnumber(val);
  arg->type=number_a;
}

///////////////////////////////////////////////////////
void make_booloperand(vmarg* arg,unsigned val){
  arg->val=val;
  arg->type=bool_a;
}

///////////////////////////////////////////////////////
void make_retvaloperand(vmarg* arg){
  arg->type=retval_a;
}

///////////////////////////////////////////////////////
void generate(enum vmopcode opcode,quad* q){

  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *arg2;
  arg2=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  t->opcode=opcode;

  if(q->arg1!=NULL){
    make_operand(q->arg1,arg1);
    t->arg1=arg1;
  }

  if(q->arg2!=NULL){
    make_operand(q->arg2,arg2);
    t->arg2=arg2;
  }

  if(q->result!=NULL){
    make_operand(q->result,result);
    t->result=result;
  }

  t->srcLine=q->line;
  q->taddress=currInstr;

  emit_ins(t);
}

///////////////////////////////////////////////////////
void generate_relational (enum vmopcode opcode, quad* q) {

  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *arg2;
  arg2=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  t->opcode=opcode;

  if(q->arg1!=NULL){
    make_operand(q->arg1,arg1);
    t->arg1=arg1;
  }

  if(q->arg2!=NULL){
    make_operand(q->arg2,arg2);
    t->arg2=arg2;
  }
  t->result=result;
  t->result->type=label_a;

  if((q->label)<(quad_runner)){
    t->result->val=((getQuads()+(q->label))->taddress);
  }
  else{
    add_incomplete_jump(currInstr,q->label);
  }
  t->srcLine=q->line;
  q->taddress = currInstr;
  emit_ins(t);
}

///////////////////////////////////////////////////////
void insert_ret_list(unsigned l,SymbolTableEntry* f){
  struct return_list *new_ret;
  new_ret=(struct return_list*)malloc(sizeof(struct return_list));
  if(new_ret==NULL){
    printf("ERROR: OUT_OF_MEMORY\n");
    return;
  }
  new_ret->label=l;

  if(f->ret_head==NULL){
    f->ret_head=new_ret;
  }
  else{
    new_ret->next=f->ret_head;
    f->ret_head=new_ret;
  }
}

///////////////////////////////////////////////////////
void backpatch_ret_list(unsigned l,SymbolTableEntry* f){
  struct return_list *temp_ret;
  temp_ret=(struct return_list*)malloc(sizeof(struct return_list));

  temp_ret=f->ret_head;
  while(temp_ret!=NULL){
    ((instructions+temp_ret->label)->result)->val=l;
    temp_ret=temp_ret->next;
  }
}

///////////////////////////////////////////////////////
void add_incomplete_jump(unsigned instrNo,unsigned iaddress){
  struct incomplete_jump *new_jump;
  new_jump=(struct incomplete_jump*)malloc(sizeof(struct incomplete_jump));

  if(new_jump==NULL){
    printf("ERROR: OUT_OF_MEMORY\n");
    return;
  }
  new_jump->instrNo=instrNo;
  new_jump->iaddress=iaddress;

  if(ij_head==NULL){
    ij_head=new_jump;
  }
  else{
    new_jump->next=ij_head;
    ij_head=new_jump;
  }
}

///////////////////////////////////////////////////////
void patch_incomplete_jumps(){
  struct incomplete_jump* j_temp;
  j_temp=ij_head;
  while(j_temp!=NULL){
    if(j_temp->iaddress==currQuad){
      ((instructions+j_temp->instrNo)->result->val)=currInstr-1;
    }
    else{
      ((instructions+j_temp->instrNo)->result->val)=((getQuads()+j_temp->iaddress)->taddress);
    }
    j_temp=j_temp->next;
  }
}

///////////////////////////////////////////////////////
extern void generate_ADD(quad* temp){
  generate(add_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_SUB(quad* temp){
  generate(sub_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_MUL(quad* temp){
  generate(mul_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_DIV(quad* temp){
  generate(div_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_MOD(quad* temp){
  generate(mod_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_NEWTABLE(quad* temp){
  generate(newtable_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_TABLEGETELEM(quad* temp){
  generate(tablegetelem_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_TABLESETELEM(quad* temp){
  generate(tablesetelem_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_ASSIGN(quad* temp){
  generate(assign_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_NOP(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  t->opcode=nop_v;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_JUMP(quad* temp){
  generate_relational(jump_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_EQ(quad* temp){
  generate_relational(jeq_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_NOTEQ(quad* temp){
  generate_relational(jne_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_GREATER(quad* temp){
  generate_relational(jgt_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_GREATEREQ(quad* temp){
  generate_relational(jge_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_LESS(quad* temp){
  generate_relational(jlt_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_IF_LESSEQ(quad* temp){
  generate_relational(jle_v,temp);
}

///////////////////////////////////////////////////////
extern void generate_NOT(quad* temp){
  //etoimo apo fash 3 ,merikh apotimish
}

///////////////////////////////////////////////////////
extern void generate_PARAM(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));

  temp->taddress = currInstr;
  t->opcode = pusharg_v;

  if(temp->arg1!=NULL){
    make_operand(temp->arg1,arg1);
    t->arg1=arg1;
  }
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_CALL(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  temp->taddress = currInstr;
  t->opcode = call_v;
  if(temp->result!=NULL){
    make_operand(temp->result,result);
    t->arg1=result;
  }
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_UMINUS(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *arg2;
  arg2=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  t->opcode = mul_v;

  make_operand(temp->arg1,arg1);
  t->arg1=arg1;

  arg2->type=number_a;
  arg2->val=consts_newnumber(-1);
  t->arg2=arg2;

  make_operand(temp->result,result);
  t->result=result;

  temp->taddress = currInstr;
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_GETRETVAL(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  temp->taddress = currInstr;
  t->opcode = assign_v;

  if(temp->result!=NULL){
    make_operand(temp->result,result);
    t->result=result;
  }

  make_retvaloperand(arg1);
  t->arg1=arg1;

  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_FUNCSTART(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct SymbolTableEntry *f;
  f=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

  f=temp->result->sym;
  f->taddress=currInstr;
  temp->taddress=currInstr;

  userfuncs_newfunc(f);
  push_func(f);

  t->opcode=funcenter_v;

  if(temp->result!=NULL){
    make_operand(temp->result,result);
    t->result=result;
  }
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_RETURN(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  temp->taddress=currInstr;

  t->opcode=assign_v;
  make_retvaloperand(result);
  t->result=result;
  if(temp->arg1!=NULL){
    make_operand(temp->arg1,arg1);
    t->arg1=arg1;
  }
  t->srcLine=temp->line;
  emit_ins(t);


  insert_ret_list(currInstr,top_func());

  t->opcode=jump_v;
  t->arg1=NULL;
  t->arg2=NULL;
  t->result->type=label_a;
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_FUNCEND(quad* temp){
  struct instruction *t;
  t=(struct instruction*)malloc(sizeof(struct instruction));
  struct vmarg *result;
  result=(struct vmarg*)malloc(sizeof(struct vmarg));

  backpatch_ret_list(currInstr,pop_func());

  temp->taddress=currInstr;
  t->opcode=funcexit_v;

  if(temp->result!=NULL){
    make_operand(temp->result,result);
    t->result=result;
  }
  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
char* return_instraction_op(int op){
  switch (op) {
    case 0 : return "assign_v";
    case 1 : return "add_v";
    case 2 : return "sub_v";
    case 3 : return "mul_v";
    case 4 : return "div_v";
    case 5 : return "mod_v";
    case 6 : return "uminus_v";
    case 7 : return "and_v";
    case 8 : return "or_v";
    case 9 : return "not_v";
    case 10 : return "jeq_v";
    case 11 : return "jne_v";
    case 12 : return "jle_v";
    case 13 : return "jge_v";
    case 14 : return "jlt_v";
    case 15 : return "jgt_v";
    case 16 : return "call_v";
    case 17 : return "pusharg_v";
    case 18 : return "funcenter_v";
    case 19 : return "funcexit_v";
    case 20 : return "newtable_v";
    case 21 : return "tablegetelem_v";
    case 22 : return "tablesetelem_v";
    case 23 : return "nop_v";
    case 24 : return "jump_v";
    default :return "empty instruction";
  }
}

///////////////////////////////////////////////////////
void print_instructions_table(FILE* out){
  fprintf(out,"\n");
  fprintf(out,"######################################################################################################################################\n\n");
	fprintf(out,"Insruction #\t\tOpcode\t\t\tResult\t\t\tArg1\t\t\tArg2\t\t\tSource line\n");
  for (int i=0;i<currInstr;i++){
    int j=strlen(return_instraction_op((instructions+i)->opcode));
    fprintf(out,"_____________________________________________________________________________________________________________________________________\n");

    fprintf(out,"# %d\t\t\t%s",i,return_instraction_op((instructions+i)->opcode));

    if((instructions+i)->result!=NULL){
      if(j<8){
        fprintf(out,"\t\t\t%d",((instructions+i)->result)->val);
      }
      else{
        fprintf(out,"\t\t%d",((instructions+i)->result)->val);
      }
    }
    else{
      if(j<8){
        fprintf(out,"\t\t\t");
      }
      else{
        fprintf(out,"\t\t");
      }
    }

    if((instructions+i)->arg1!=NULL){
      fprintf(out,"\t\t\t%d",((instructions+i)->arg1)->val);
    }

    if((instructions+i)->arg2!=NULL){
      fprintf(out,"\t\t\t%d",((instructions+i)->arg2)->val);
    }

    if((instructions+i)->arg1==NULL){
      fprintf(out,"\t\t\t\t\t\t\t\t\t%d",(instructions+i)->srcLine);
    }
    else if((instructions+i)->arg2==NULL){
      fprintf(out,"\t\t\t\t\t\t%d",(instructions+i)->srcLine);
    }
    else{
      fprintf(out,"\t\t\t%d",(instructions+i)->srcLine);
    }
    fprintf(out,"\n");
  }
  fprintf(out,"\n");
  fprintf(out,"######################################################################################################################################\n\n");
}

///////////////////////////////////////////////////////
void push_func(SymbolTableEntry* f){
  struct func_stack_entry* new_entry;
  new_entry=(struct func_stack_entry*)malloc(sizeof(struct func_stack_entry));

  if(new_entry==NULL){
    printf("OUT_OF_MEMORY\n");
    exit(0);
  }
  new_entry->function=f;

  if(f_stack==NULL){
    f_stack=new_entry;
  }
  else{
    new_entry->next=f_stack;
    f_stack=new_entry;
  }
}

///////////////////////////////////////////////////////
SymbolTableEntry* pop_func(){
  if(f_stack==NULL){
    printf("Empty func_stack\n");
    return NULL;
  }
  else{
    struct SymbolTableEntry* temp;
    temp=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

    temp=f_stack->function;
    f_stack=f_stack->next;
    return temp;
  }
}

///////////////////////////////////////////////////////
SymbolTableEntry* top_func(){
  if(f_stack==NULL){
    printf("Empty func_stack\n");
    return NULL;
  }
  else{
    return f_stack->function;
  }
}

///////////////////////////////////////////////////////
void nop_emit(){
  struct instruction *nop;
  nop=(struct instruction*)malloc(sizeof(struct instruction));
  nop->opcode=nop_v;
  emit_ins(nop);
  return;
}

///////////////////////////////////////////////////////
FILE* convert_to_binary(){

  FILE* file;

  file=fopen("output","wb");

    if(file!=NULL){

      struct instruction ins;
      struct userFunc us;
      struct strings lb;
      struct strings st;
      double n;

      //instructions talbe
      for (int i=0;i<currInstr;i++){
        ins=*(instructions+i);
        fwrite(&ins,sizeof(struct instruction),1,file);
      }

      //numbers
      for (int j=0; j<curr_nums;j++){
        n=*(numbers+j);
        fwrite(&n,sizeof(double),1,file);
      }

      //lib_funcs
      for (int j=0; j<curr_lib_funcs;j++){
        lb=*(lib_funcs+j);
        fwrite(&lb,sizeof(struct strings),1,file);
      }

      //user_funcs
      for (int j=0; j<curr_funcs;j++){
        us=*(user_funcs+j);
        fwrite(&us,sizeof(struct userFunc),1,file);
      }

      //strings
      for (int j=0; j<curr_strings;j++){
        st=*(string_consts+j);
        fwrite(&st,sizeof(struct strings),1,file);
      }

      fclose(file);
    }
  }
