#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Function_stack.h"


struct Function_stack* f_stack_head=NULL;

///////////////////////////////////////////////////////////////////////

Function_stack* f_pop(){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));
  assert(temp!=NULL);

  if(f_stack_head==NULL){
    printf("!!!!!!!!!! Empty Stack !!!!!!!!!!\n");
    return NULL;
  }
  else{
    temp=f_stack_head;
    f_stack_head=temp->next;
    return temp;
  }
}
///////////////////////////////////////////////////////////////////////
void f_push(char* c,int label){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));
  assert(temp!=NULL);
  if(f_stack_head==NULL){
    f_stack_head=temp;
    f_stack_head->name=c;
    f_stack_head->label=label;
  }
  else{
    temp->name=c;
    temp->label=label;
    temp->next=f_stack_head;
    f_stack_head=temp;
  }
}
/////////////////////////////////////////////////////////////
void f_print_stack(){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));
  temp=f_stack_head;
  assert(temp!=NULL);
  while(temp!=NULL){
    printf("From Function Stack:  %s\n",temp->name);
    temp=temp->next;
  }
}
