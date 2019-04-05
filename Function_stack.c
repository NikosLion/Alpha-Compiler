#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Function_stack.h"


struct Function_stack* f_stack_head=NULL;

char* f_pop(){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));
  assert(temp!=NULL);

  char* ret;

  if(f_stack_head==NULL){
    printf("Stack Empty !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    return (char*)-1;
  }
  else{
    temp=f_stack_head;
    f_stack_head=temp->next;
    ret=temp->name;
    free(temp);
    return ret;
  }
}
///////////////////////////////////////////////////////////
void f_push(char* c){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));

  assert(temp!=NULL);
  if(f_stack_head==NULL){
    f_stack_head=temp;
    f_stack_head->name=c;
  }
  else{
    temp->name=c;
    temp->next=f_stack_head;
    f_stack_head=temp;
  }
}
/////////////////////////////////////////////////////////////
void f_print_stack(){
  struct Function_stack*  temp;
  temp=(struct Function_stack*)malloc(sizeof(struct Function_stack));

  assert(temp!=NULL);
  temp=f_stack_head;
  while(temp!=NULL){
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   %s \n",temp->name);
    temp=temp->next;
  }
}
