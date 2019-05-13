#include "Vm_args.h"
#include <stdlib.h>

unsigned consts_newstring(char* s){}
unsigned consts_newnumber(double n){}
unsigned libfuncs_newused(char* s){}
unsigned userfuncs_newfunc(SymbolTableEntry* sym){}

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
      //arg->type=libraryfunc_a;
      //arg->val=libfuncs_newused(e->sym->name);
      break;
    }
    default:exit(0);
  }
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
      ((instructions+j_temp->instrNo)->result->val)=currInstr;
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
  struct vmarg *arg1;
  arg1=(struct vmarg*)malloc(sizeof(struct vmarg));

  temp->taddress = currInstr;
  t->opcode = call_v;
  if(temp->arg1!=NULL){
    make_operand(temp->arg1,arg1);
    t->arg1=arg1;
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

  //make_retvaloperand(&t.arg1);

  t->srcLine=temp->line;
  emit_ins(t);
}

///////////////////////////////////////////////////////
extern void generate_FUNCSTART(quad* temp){

}
///////////////////////////////////////////////////////
extern void generate_RETURN(quad* temp){

}
///////////////////////////////////////////////////////
extern void generate_FUNCEND(quad* temp){

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

    if(j<8){
      fprintf(out,"\t\t\t%d",((instructions+i)->result)->val);
    }
    else{
      fprintf(out,"\t\t%d",((instructions+i)->result)->val);
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
