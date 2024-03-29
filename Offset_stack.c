#include "Offset_stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct scopeSpace_stack* stack_head=NULL;

/////////////////////////////////////////////////////////////

unsigned pop(){
  struct scopeSpace_stack*  temp;
  temp=(struct scopeSpace_stack*)malloc(sizeof(struct scopeSpace_stack));
  assert(temp!=NULL);
  unsigned ret;

  if(stack_head==NULL){
    printf("!!!!!!!!!! Empty Stack !!!!!!!!!!\n");
    return -1;
  }
  else{
    temp=stack_head;
    ret=temp->offset;
    stack_head=temp->next;
    return ret;
  }
}

///////////////////////////////////////////////////////////
void push(unsigned offset){
  struct scopeSpace_stack*  temp;
  temp=(struct scopeSpace_stack*)malloc(sizeof(struct scopeSpace_stack));
  assert(temp!=NULL);
  if(stack_head==NULL){
    temp->offset==offset;
    stack_head=temp;
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
    printf("From Offset Stack:  %d\n",temp->offset);
    temp=temp->next;
  }
}
