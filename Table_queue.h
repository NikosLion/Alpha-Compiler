
#include "Quads.h"

typedef struct queue_node{
  expr *data;
  struct queue_node *next;
}queue_node;

void init_queue();
void enqueue_table_queue(expr* new_table_item);
expr* dequeue_table_queue();
int is_empty_queue();
void reset_queue();
