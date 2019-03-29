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
