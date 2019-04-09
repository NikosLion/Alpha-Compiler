#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "quads.h"


////////////////////////////////////////////////////////////////////////////
quad *if_quad;

quad* quads = (quad*) 0;

struct expr *temp_expr;
struct SymbolTableEntry *t_sym;


void emit(enum iopcode op,expr* arg1,expr* arg2,expr* result,int label,unsigned line){

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
    default :return "empty quad";
  }
}

////////////////////////////////////////////
void print_quads(FILE* out){
  fprintf(out,"######################################################################################################################################\n\n");
	fprintf(out,"Quad#\t\tOpcode\t\t\tResult\t\t\tArg1\t\t\tArg2\t\t\tLabel\t\tLine\n");
  int i=0;
	for(i;i<currQuad;i++){
    fprintf(out,"_____________________________________________________________________________________________________________________________________\n");
		struct quad* temp=quads+i;
    if(temp==NULL){
      fprintf(out,"Empty Quads\n");
      break;
    }
    int j=strlen(return_op(temp->op));
    if(j<8){
      fprintf(out,"# %d\t\t%s\t\t\t",i,return_op(temp->op));
    }
    else{
      fprintf(out,"# %d\t\t%s\t\t",i,return_op(temp->op));
    }

    if(temp->result!=NULL){
      fprintf(out,"%s\t\t\t",temp->result->sym->name);
    }
    else{
      fprintf(out,"\t\t\t");
    }
    if(temp->arg1!=NULL){

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
      else if(temp->arg1->type==constnum_e){
        if(temp->arg1->int_real==0){
          fprintf(out,"%f\t\t\t",temp->arg1->value.realValue);
        }
        else if(temp->arg1->int_real==1){
          fprintf(out,"%d\t\t\t",temp->arg1->value.intValue);
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
      else if(temp->arg2->type==constnum_e){
        if(temp->arg2->int_real==0){
          fprintf(out,"%f\t\t\t",temp->arg2->value.realValue);
        }
        else if(temp->arg2->int_real==1){
          fprintf(out,"%d\t\t\t",temp->arg2->value.intValue);
        }
      }
      else{
        fprintf(out,"%s\t\t\t",temp->arg2->sym->name);
      }
    }
    else{
      fprintf(out,"\t\t\t");
    }
    if((temp->op==if_eq)||(temp->op==if_noteq)||(temp->op==if_lesseq)||(temp->op==if_greatereq)||(temp->op==if_less)||(temp->op==if_greater)||(temp->op==jump)){
      fprintf(out,"%d\t\t",temp->label);
    }
    else{
      fprintf(out,"\t\t");
    }
    fprintf(out,"%d\t\n",temp->line);

  }
  fprintf(out,"\n######################################################################################################################################\n\n");
}


///////////////////////////////////////////////////////////////
int make_bool(struct expr *expr){
  if(expr==NULL){
    return 0;
  }
  else if(expr->type==constnum_e){
    if(expr->int_real==1){
      if(expr->value.intValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
    else if(expr->int_real==0){
      if(expr->value.realValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
  }
  else if(expr->type==conststring_e){
    if(expr->value.stringValue!=""){
      return 1;
    }
    else{
      return 0;
    }
  }
  else if(expr->type==constbool_e){
    if(expr->value.boolean==1){
      return 1;
    }
    else{
      return 0;
    }
  }
  else if(expr->type==nil_e){
    return 0;
  }
  else if(expr->type==programfunc_e){
    return 1;
  }
  else if(expr->type==libraryfunc_e){
    return 1;
  }
  else if(expr->type==newtable_e){
    return 1;
  }
  else if(expr->type==arithexpr_e){
    if(expr->int_real==1){
      if(expr->value.intValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
    else if(expr->int_real==0){
      if(expr->value.realValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
  }
  else if(expr->type==boolexpr_e){
    if(expr->value.boolean==1){
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(expr->type==var_e){
    if(expr->int_real==1){
      if(expr->value.intValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
    else if(expr->int_real==0){
      if(expr->value.realValue!=0){
        return 1;
      }
      else {
        return 0;
      }
    }
    else if(expr->int_real==-1){
      if(expr->value.stringValue!=""){
        return 1;
      }
      else{
        return 0;
      }
    }
    else if(expr->int_real==-2){
      if(expr->value.boolean==1){
        return 1;
      }
      else {
        return 0;
      }
    }
    else {
      return -123; ///ki alles periptwseis gia pinakes
    }
  }
}
////////////////////////////////////////////////////////////////////////

expr* make_if_quad(int label, expr* temp){

  temp_expr=(struct expr*)malloc(sizeof(struct expr));
  t_sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
  temp_expr->sym=t_sym;
  if(temp->type==var_e){
    temp_expr->sym->name=temp->sym->name;
  }
  else if(temp->type==constnum_e){
    if(temp->int_real==1){
      char s[20];
      sprintf(s,"%d",temp->value.intValue);
      temp_expr->sym->name=s;
    }
    else if(temp->int_real==0){
      char s[20];
      sprintf(s,"%d",temp->value.intValue);
      temp_expr->sym->name=s;
    }
  }
  else if(temp->type==conststring_e){
    temp_expr->sym->name=temp->value.stringValue;
  }
  else if(temp->type==constbool_e){
    if(temp->value.boolean==0){
      temp_expr->sym->name="_false";
    }
    else if(temp->value.boolean==1){
      temp_expr->sym->name="_true";
    }
  }
  else if(temp->type==nil_e){
    temp_expr->sym->name="_false";
  }
  temp_expr->value.intValue=label-1;
  return temp_expr;
}

////////////////////////////////////////////////////////////////////////

void if_backpatch(expr* temp){
  int i=currQuad-2;
  struct quad* t_q=quads+i;
  struct quad* base_quad=quads+(currQuad-1);

  //to i mas deixnei ton ari8mo tou quad mesa ston pinaka

  //fix jump labels for true-false
  while(i>=0){
    if(t_q->op==assign){
      if((t_q->result->sym->name==temp->sym->name) && (t_q->arg1->int_real==-2) && (t_q->arg1->value.boolean==0)){
        (t_q+1)->label=temp->value.intValue;
      }
      else if((t_q->result->sym->name==temp->sym->name) && (t_q->arg1->int_real==-2) && (t_q->arg1->value.boolean==1)){
        (t_q+1)->label=temp->value.intValue;
        ///////////////////////////////////
        //periptwsh or
        if(t_q->result->int_real==-5){
          base_quad->label=i+4;
        }
        //periptwsh and
        else if(t_q->result->int_real==-6){
          base_quad->label=i+4;
        }
        //ta upoloipa
        else{
          base_quad->label=i+2;
        }
        //////////////////////////////////
      }
    }
    i--;
    t_q=quads+i;
  }
  i=currQuad-2;
  t_q=quads+i;
  base_quad=quads+(currQuad-1);

  //fix if_eq labels
  while(i>=0){
    if(t_q->op==if_eq){
       if((t_q->arg1->sym->name==temp->sym->name) && (t_q->arg2->int_real==-2) && (t_q->arg2->value.boolean==1)){
         int j=i-1;
         struct quad* t_j=quads+j;
         while(j>=0){
           if(t_j->op==assign){
             if((t_j->result->sym->name==temp->sym->name) && (t_j->arg1->int_real==-2) && (t_j->arg1->value.boolean==1)){
               //periptwsh or
               if(t_j->result->int_real==-5){
                 (t_q)->label=j+4;
               }
               //periptwsh and
               else if(t_j->result->int_real==-6){
                 (t_q)->label=j+4;
               }
               //ta upoloipa
               else{
                 (t_q)->label=j+2;
               }
             }
           }
           j--;
           t_j=quads+j;
         }
       }
     }
     i--;
     t_q=quads+i;
  }

  //fix jump labels for jump quads after true
  i=currQuad;
  t_q=quads+i;
  base_quad=quads+(currQuad);
  while(i>=0){
    if(t_q->op==if_eq){
       if((t_q->arg1->sym->name==temp->sym->name) && (t_q->arg2->int_real==-2) && (t_q->arg2->value.boolean==1)){
         (t_q-1)->label=currQuad;
       }
    }
    i--;
    t_q=quads+i;
  }
  return;
}
/////////////////////////////////////////////////////////////////////////
