
#include "symtable.h"

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

enum iopcode{
  assign,   and,          if_less,      funcstart,
  add,      or,           if_greater,   funcend,
  sub,      not,          jump,         tablecreate,
  mul,      if_eq,        call,         tablegetelem,
  Div,      if_noteq,     param,        tablesetelem,
  mod,      if_lesseq,    Return,
  uminus,   if_geatereq,  getretval
};


typedef struct quad{
  enum iopcode op;
  SymbolTableEntry* result;
  SymbolTableEntry* arg1;
  SymbolTableEntry* arg2;
  unsigned label;
  unsigned line;
}quad;

quad* quads = (quad*) 0;
void emit(enum iopcode op,SymbolTableEntry* arg1,SymbolTableEntry* arg2,SymbolTableEntry* result,unsigned label,unsigned line);
void expand();
