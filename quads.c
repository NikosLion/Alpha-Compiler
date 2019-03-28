#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "quads.h"

unsigned total=0;
unsigned int currQuad=0;

void emit(enum iopcode op,SymbolTableEntry* arg1,SymbolTableEntry* arg2,SymbolTableEntry* result,unsigned label,unsigned line){

  if(currQuad == total){
    expand();
  }
  quad* p = quads+currQuad;
  p->arg1=arg1;
  p->arg2=arg2;
  p->result=result;
  p->label=label;
  p->line=line;
  currQuad++;
}

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
