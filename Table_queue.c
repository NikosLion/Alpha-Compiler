
#include <stdlib.h>
#include "Table_queue.h"

struct queue_node *front;
struct queue_node *rear;

///////////////////////////////////////////////////////
void init_queue(){
  front=(struct queue_node*)malloc(sizeof(struct queue_node));
  rear=(struct queue_node*)malloc(sizeof(struct queue_node));
  front=NULL;
  rear=NULL;
}

///////////////////////////////////////////////////////
void enqueue_table_queue(expr* new_table_item){
  queue_node *new_entry;
  new_entry=malloc(sizeof(queue_node));

  if(new_entry==NULL){
    printf("ERROR: Out of memory!\n");
    exit(0);
  }

  new_entry->data=new_table_item;
  new_entry->next=NULL;

  if(rear==NULL){
    rear=new_entry;
    front=rear;
  }
  else{
    rear->next=new_entry;
    rear=new_entry;
  }
}

///////////////////////////////////////////////////////
expr* dequeue_table_queue(){
  queue_node *temp;
  struct expr *data;
  data=malloc(sizeof(struct expr));

  data=front->data;
  temp=front;
  front=front->next;
  if(front==NULL){
    rear=NULL;
  }
  free(temp);
  return data;
}

///////////////////////////////////////////////////////
int is_empty_queue(){
  if(front==rear){
    if(front==NULL){
      return 1;
    }
  }
  else{
    return 0;
  }
}

///////////////////////////////////////////////////////
void reset_queue(){
  front=NULL;
  rear=NULL;
}
