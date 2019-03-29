#include "Stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct scopeSpace_stack* stack_head=NULL;

unsigned pop(){
  struct scopeSpace_stack*  temp;
  temp=(struct scopeSpace_stack*)malloc(sizeof(struct scopeSpace_stack));
  assert(temp!=NULL);
  unsigned ret;

  if(stack_head==NULL){
    printf("Stack Empty !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    return -1;
  }
  else{
    temp=stack_head;
    stack_head=temp->next;
    ret=temp->offset;
    free(temp);
    return ret;
  }
}
///////////////////////////////////////////////////////////
void push(unsigned offset){
  struct scopeSpace_stack*  temp;
  temp=(struct scopeSpace_stack*)malloc(sizeof(struct scopeSpace_stack));
  assert(temp!=NULL);
  if(stack_head==NULL){
    stack_head=temp;
    stack_head->offset=offset;
  }
  else{
    temp->offset=offset;
    temp->next=stack_head;
    stack_head=temp;
  }
}
/////////////////////////////////////////////////////////////
void print_stack(){
  struct scopeSpace_stack*  temp;
  temp=(struct scopeSpace_stack*)malloc(sizeof(struct scopeSpace_stack));
  assert(temp!=NULL);
  temp=stack_head;
  while(temp!=NULL){
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   %d \n",temp->offset);
    temp=temp->next;
  }
}
