#include "Vm_args.h"
#include <stdlib.h>

unsigned consts_newstring(char* s){}
unsigned consts_newnumber(double n){}
unsigned libfuncs_newused(char* s){}
unsigned userfuncs_newfunc(SymbolTableEntry* sym){}

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

///////////////////////////////////////////////////////
void generate (){
  for(unsigned i=0;i<currQuad;++i){
    (*generators[(getQuads()+i)->op])(getQuads()+i);
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
        case programvar:  arg->type=global_a; break;
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
      arg->val=e->sym->taddress;
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
extern void generate_ADD(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_SUB(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_MUL(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_DIV(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_MOD(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_NEWTABLE(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_TABLEGETELEM(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_TABLESETELEM(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_ASSIGN(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_NOP(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_JUMP(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_EQ(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_NOTEQ(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_GREATER(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_GREATEREQ(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_LESS(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_IF_LESSEQ(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_NOT(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_PARAM(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_CALL(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_UMINUS(quad* temp){

}

///////////////////////////////////////////////////////
extern void generate_GETRETVAL(quad* temp){

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
