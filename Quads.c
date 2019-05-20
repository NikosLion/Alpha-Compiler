#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "Quads.h"


///////////////////////////////////////////////////////
quad *if_quad;

quad* quads = (quad*) 0;

struct expr *temp_expr;
struct SymbolTableEntry *t_sym;
struct func_jump* func_jump_head=NULL;

///////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////
unsigned currScopeOffset(void){
  switch (currScopeSpace()){
    case programvar : return programVarOffset;
    case functionlocal : return functionLocalOffset;
    case formalarg :  return  formalArgOffset;
    default: assert(0);
  }
}
///////////////////////////////////////////////////////
void incCurrScopeOffset(void){
  switch (currScopeSpace()){
    case programvar : ++programVarOffset; break;
    case functionlocal : ++functionLocalOffset; break;
    case formalarg : ++formalArgOffset; break;
  }
}

///////////////////////////////////////////////////////
void enterScopeSpace(void){
  ++scopeSpaceCounter;
}

///////////////////////////////////////////////////////
void exitScopeSpace(void){
  assert(scopeSpaceCounter>1);
  --scopeSpaceCounter;
}

///////////////////////////////////////////////////////
void restoreCurScopeOffset(unsigned old_offset){
  switch (currScopeSpace()){
    case programvar : programVarOffset=old_offset; break;
    case functionlocal : functionLocalOffset=old_offset; break;
    case formalarg : formalArgOffset=old_offset; break;
  }
}

///////////////////////////////////////////////////////
char* return_op(int op){
  switch (op) {
    case 0 : return "add";
    case 1 : return "sub";
    case 2 : return "mul";
    case 3 : return "Div";
    case 4 : return "mod";
    case 5 : return "tablecreate";
    case 6 : return "tablegetelem";
    case 7 : return "tablesetelem";
    case 8 : return "assign";
    case 9 : return "jump";
    case 10 : return "if_eq";
    case 11 : return "if_noteq";
    case 12 : return "if_greater";
    case 13 : return "if_eq";
    case 14 : return "if_less";
    case 15 : return "if_lesseq";
    case 16 : return "not";
    case 17 : return "param";
    case 18 : return "call";
    case 19 : return "uminus";
    case 20 : return "getretval";
    case 21 : return "funcstart";
    case 22 : return "Return";
    case 23 : return "funcend";
    default :return "empty quad";
  }
}

///////////////////////////////////////////////////////
void print_quads(FILE* out){
  fprintf(out,"######################################################################################################################################\n\n");
	fprintf(out,"Quad #\t\tOpcode\t\t\tResult\t\t\tArg1\t\t\tArg2\t\t\tLabel\t\tLine\n");
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
      else if(temp->arg1->type==boolexpr_e){
        fprintf(out,"%s\t\t\t",temp->arg1->sym->name);
      }
      else if(temp->arg1->type==tableitem_e){
        fprintf(out,"%s\t\t\t",temp->arg1->sym->name);
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
      else if(temp->arg2->type==boolexpr_e){
        //fprintf(out,"%s\t\t\t",temp->arg2->sym->name);
      }
      else if(temp->arg2->type==tableitem_e){
        fprintf(out,"%s\t\t\t",temp->arg2->sym->name);
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

///////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////
void insert_tf_list(expr* dest,int list,int label){
  struct tf_node *temp;
  struct tf_node *prev;
  struct tf_node *new_entry;
  new_entry=(struct tf_node*)malloc(sizeof(struct tf_node));

  if(new_entry==NULL){
      printf("ERROR: OUT_OF_MEMORY\n");
      return;
  }

  new_entry->label=label;
  new_entry->next=NULL;

  if(list==0){
    if(dest->false_list==NULL){
      dest->false_list=new_entry;
      dest->false_list->next=NULL;
    }
    else{
      temp=dest->false_list;
      prev=temp;
      while(temp!=NULL){
        prev=temp;
        temp=temp->next;
      }
      prev->next=new_entry;
    }
  }
  else if(list==1){
    if(dest->true_list==NULL){
      dest->true_list=new_entry;
      dest->true_list->next=NULL;
    }
    else{
      temp=dest->true_list;
      prev=temp;
      while(temp!=NULL){
        prev=temp;
        temp=temp->next;
      }
      prev->next=new_entry;
    }

  }
  return;
}

///////////////////////////////////////////////////////
void merge_tf_list(expr* left,expr* right,expr* dest,int list){
  tf_node* temp_left;
  tf_node* temp_right;

  if(list==1){
    temp_left=left->true_list;
    temp_right=right->true_list;
  }
  else if(list==0){
    temp_left=left->false_list;
    temp_right=right->false_list;
  }

  while(temp_left!=NULL){
    insert_tf_list(dest,list,temp_left->label);
    temp_left=temp_left->next;
  }
  while(temp_right!=NULL){
    insert_tf_list(dest,list,temp_right->label);
    temp_right=temp_right->next;
  }
}

///////////////////////////////////////////////////////
void backpatch(expr* patched,int patcher,int list_to_patch){
  tf_node* temp;
  struct quad* temp_quad=quads;

  if(list_to_patch==0){
    temp=patched->false_list;
  }
  else if(list_to_patch==1){
    temp=patched->true_list;
  }
  while(temp!=NULL){
    (temp_quad+(temp->label))->label=patcher;
    temp=temp->next;
  }
}

///////////////////////////////////////////////////////
void insert_jump_list(int label){

  struct jump_after_true *new_entry;
  new_entry=(struct jump_after_true*)malloc(sizeof(struct jump_after_true));

  if(new_entry==NULL){
    printf("ERROR: OUT_OF_MEMORY\n");
    return;
  }

  new_entry->label=label;

  if(jump_head==NULL){
    jump_head=new_entry;
  }
  else{
    new_entry->next=jump_head;
    jump_head=new_entry;
  }
}

///////////////////////////////////////////////////////
void backpatch_jat(int label){
  struct jump_after_true *ptr;
  ptr=jump_head;

  if(ptr==NULL){return;}
  while (ptr!=NULL) {
    int i=0;
    for(i;i<currQuad;i++){

      if(i==ptr->label){

        (quads+i)->label=label;
        break;
      }
    }
    ptr=ptr->next;
  }
}

///////////////////////////////////////////////////////
void backpatch_rat(int cur,int label){
    (quads+cur-1)->label=label+2;
}

///////////////////////////////////////////////////////
void backpatch_jaf(int cur,int lab){
  (quads+cur)->label=lab;
}

///////////////////////////////////////////////////////
void backpatch_else(int cur,int label){
  (quads+cur)->label=label;
}

///////////////////////////////////////////////////////
void insert_break_list(int label){

  struct tf_node *new_entry;
  new_entry=(struct tf_node*)malloc(sizeof(struct tf_node));

  if(new_entry==NULL){
    printf("ERROR: OUT_OF_MEMORY\n");
    return;
  }

  new_entry->label=label;

  if(break_head==NULL){
    break_head=new_entry;
  }
  else{
    new_entry->next=break_head;
    break_head=new_entry;
  }
}

///////////////////////////////////////////////////////
void backpatch_break(int label){

  struct tf_node *ptr;
  ptr=break_head;

  if(ptr==NULL){return;}
  while (ptr!=NULL) {
    int i=0;
    for(i;i<currQuad;i++){
      if(i==ptr->label){
        (quads+i)->label=label;
        break;
      }
    }
    ptr=ptr->next;
  }
  break_head=NULL;
}

///////////////////////////////////////////////////////
void insert_continue_list(int label){
  struct tf_node *new_entry;
  new_entry=(struct tf_node*)malloc(sizeof(struct tf_node));

  if(new_entry==NULL){
    printf("ERROR: OUT_OF_MEMORY\n");
    return;
  }

  new_entry->label=label;

  if(continue_head==NULL){
    continue_head=new_entry;
  }
  else{
    new_entry->next=continue_head;
    continue_head=new_entry;
  }
}

///////////////////////////////////////////////////////
void backpatch_continue(int label){

  struct tf_node *ptr;
  ptr=continue_head;

  if(ptr==NULL){return;}
  while (ptr!=NULL) {
    int i=0;
    for(i;i<currQuad;i++){
      if(i==ptr->label){
        (quads+i)->label=label;
        break;
      }
    }
    ptr=ptr->next;
  }
  continue_head=NULL;
}

///////////////////////////////////////////////////////
quad* getQuads(){
  return quads;
}

///////////////////////////////////////////////////////
void insert_funcstart_list(int label){
  struct func_jump*  temp;
  temp=(struct func_jump*)malloc(sizeof(struct func_jump));
  assert(temp!=NULL);
  if(func_jump_head==NULL){
    func_jump_head=temp;
    func_jump_head->label=label;
  }
  else{
    temp->label=label;
    temp->next=func_jump_head;
    func_jump_head=temp;
  }
}

///////////////////////////////////////////////////////
void backpatch_funcstart_list(int label){
  struct func_jump*  temp;
  temp=(struct func_jump*)malloc(sizeof(struct func_jump));
  assert(temp!=NULL);
  if(func_jump_head!=NULL){
    temp=func_jump_head;
    func_jump_head=temp->next;
    (quads+(temp->label))->label=label;
  }
}
