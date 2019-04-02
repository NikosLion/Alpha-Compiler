#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "quads.h"


quad* quads = (quad*) 0;

void emit(enum iopcode op,expr* arg1,expr* arg2,expr* result,unsigned label,unsigned line){

  if(currQuad == total){
    expand();
  }
  quad* p = quads+currQuad;
  p->op=op;
  p->arg1=arg1;
  p->arg2=arg2;
  p->result=result;
  p->label=label;
  p->line=line;
  currQuad++;
}

///////////////////////////////////////
void expand(){
  assert(total == currQuad);
  quad* p=(quad*)malloc(NEW_SIZE);
  if(quads){
    memcpy(p,quads,CURR_SIZE);
    free(quads);
  }
  quads=p;
  total=total+EXPAND_SIZE;
}

///////////////////////////////////////
enum scopespace_t currScopeSpace(void){
  if(scopeSpaceCounter == 1){
    return programvar;
  }
  else if(scopeSpaceCounter % 2 == 0){
    return formalarg;
  }
  else{
    return functionlocal;
  }
}

/////////////////////////////////////
unsigned currScopeOffset(void){
  switch (currScopeSpace()){
    case programvar : return programVarOffset;
    case functionlocal : return functionLocalOffset;
    case formalarg :  return  formalArgOffset;
    default: assert(0);
  }
}
//////////////////////////////////////
void incCurrScopeOffset(void){
  switch (currScopeSpace()){
    case programvar : ++programVarOffset; break;
    case functionlocal : ++functionLocalOffset; break;
    case formalarg : ++formalArgOffset; break;
  }
}

//////////////////////////////////////
void enterScopeSpace(void){
  ++scopeSpaceCounter;
}

////////////////////////////////////
void exitScopeSpace(void){
  assert(scopeSpaceCounter>1);
  --scopeSpaceCounter;
}

/////////////////////////////////////
void restoreCurScopeOffset(unsigned old_offset){
  switch (currScopeSpace()){
    case programvar : programVarOffset=old_offset; break;
    case functionlocal : functionLocalOffset=old_offset; break;
    case formalarg : formalArgOffset=old_offset; break;
  }
}
/////////////////////////////////////////////
char* return_op(int op){
  switch (op) {
    case 0 : return "assign";
    case 1 : return "and";
    case 2 : return "if_less";
    case 3 : return "funcstart";
    case 4 : return "add";
    case 5 : return "or";
    case 6 : return "if_greater";
    case 7 : return "funcend";
    case 8 : return "sub";
    case 9 : return "not";
    case 10 : return "jump";
    case 11 : return "tablecreate";
    case 12 : return "mul";
    case 13 : return "if_eq";
    case 14 : return "call";
    case 15 : return "tablegetelem";
    case 16 : return "Div";
    case 17 : return "if_noteq";
    case 18 : return "param";
    case 19 : return "tablesetelem";
    case 20 : return "mod";
    case 21 : return "if_lesseq";
    case 22 : return "Return";
    case 23 : return "uminus";
    case 24 : return "if_geatereq";
    case 25 : return "getretval";
    default :return "empy quad";
  }
}

////////////////////////////////////////////
void print_quads(FILE* out){
	fprintf(out,"Quad#\t\t\tOpcode\t\t\tresult\t\t\targ1\t\t\targ2\t\t\tlabel\n");
  int i=0;
	for(i;i<currQuad;i++){
    fprintf(out,"\n");
		struct quad* temp=quads+i;
    if(temp==NULL){
      fprintf(out,"Empty Quads\n");
      break;
    }
    fprintf(out,"%d\t\t\t%s\t\t\t",i,return_op(temp->op));
    if(temp->result!=NULL){
      fprintf(out,"%s\t\t\t",temp->result->sym->name);
    }
    else{
      fprintf(out,"\t\t\t");
    }
    if(temp->arg1!=NULL){
      if(temp->arg1->sym==NULL){
        if(temp->arg1->type==conststring_e){
          fprintf(out,"%s\t\t\t",temp->arg1->value.stringValue);
        }
        else if(temp->arg1->type==constbool_e){
          if(temp->arg1->value.boolean==0){
            fprintf(out,"false\t\t\t");
          }
          else if(temp->arg1->value.boolean==1){
            fprintf(out,"true\t\t\t");
          }
        }
        else if(temp->arg1->type==nil_e){
          fprintf(out,"null\t\t\t");
        }
        else{
          if(temp->arg1->int_real==0){
            fprintf(out,"%f\\t\t\t",temp->arg1->value.realValue);
          }
          else if(temp->arg1->int_real==1){
            fprintf(out,"%d\t\t\t",temp->arg1->value.intValue);
          }
        }
      }
      else{
        fprintf(out,"%s\t\t\t",temp->arg1->sym->name);
      }
    }
    else{
      fprintf(out,"\t\t\t");
    }
    if(temp->arg2!=NULL){
      if(temp->arg2->sym==NULL){
        if(temp->arg2->type==conststring_e){
          fprintf(out,"%s\t\t\t",temp->arg2->value.stringValue);
        }
        else if(temp->arg2->type==constbool_e){
          if(temp->arg2->value.boolean==0){
            fprintf(out,"false\t\t\t");
          }
          else if(temp->arg2->value.boolean==1){
            fprintf(out,"true\t\t\t");
          }
        }
        else if(temp->arg2->type==nil_e){
          fprintf(out,"null\t\t\t");
        }
        else{
          if(temp->arg2->int_real==0){
            fprintf(out,"%f\t\t\t",temp->arg2->value.realValue);
          }
          else if(temp->arg2->int_real==1){
            fprintf(out,"%d\t\t\t",temp->arg2->value.intValue);
          }
        }
      }
      else{
        fprintf(out,"%s\t\t\t",temp->arg2->sym->name);
      }
    }
    else{
      fprintf(out,"\t\t\t");
    }
    if((temp->op==if_eq)||(temp->op==if_noteq)||(temp->op==if_lesseq)||(temp->op==if_geatereq)||(temp->op==if_less)||(temp->op==if_greater)||(temp->op==jump)){
      fprintf(out,"%d\t\t\t",temp->label);
    }
  }
  fprintf(out,"\n\n");
}
