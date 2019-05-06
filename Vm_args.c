#include "Vm_args.h"
#include <stdlib.h>

unsigned consts_newstring(char* s){}
unsigned consts_newnumber(double n){}
unsigned libfuncs_newused(char* s){}
unsigned userfuncs_newfunc(SymbolTableEntry* sym){}

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
