
#include "Symtable.h"
#include <stdio.h>

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

unsigned total;
int currQuad;

struct jump_after_true *jump_head;

unsigned programVarOffset;
unsigned functionLocalOffset;
unsigned formalArgOffset;
unsigned scopeSpaceCounter;

enum iopcode{
  assign,       and,          if_less,      funcstart,
  add,          or,           if_greater,   funcend,
  sub,          not,          jump,         tablecreate,
  mul,          if_eq,        call,         tablegetelem,
  Div,          if_noteq,     param,        tablesetelem,
  mod,          if_lesseq,    Return,       uminus,
  if_greatereq, getretval,    no_op
};

enum expr_t{
  var_e,            arithexpr_e,    constnum_e,
  tableitem_e,      boolexpr_e,     constbool_e,
  programfunc_e,    assignexpr_e,   conststring_e,
  libraryfunc_e,    newtable_e,     nil_e
};

typedef struct tf_node{
  int label;
  struct tf_node* next;
}tf_node;


typedef struct jump_after_true{
  int label;
  struct jump_after_true* next;
}jump_after_true;


typedef struct expr{
  enum expr_t type;
  SymbolTableEntry* sym;
  int int_real;
  union value{
    char* stringValue;
    int intValue;
    double realValue;
    int boolean;    //true 1,false 0,null 0
  }value;
  tf_node* true_list;
  tf_node* false_list;
  struct expr* index;
  struct expr* next;
}expr;


typedef struct quad{
  enum iopcode op;
  expr* result;
  expr* arg1;
  expr* arg2;
  unsigned label;
  unsigned line;
}quad;


void emit(enum iopcode op,expr* arg1,expr* arg2,expr* result,int label,unsigned line);
void expand(void);
void print_quads(FILE* out);
int make_bool(struct expr *expr);
expr* make_if_quad(int label, expr* temp);
void insert_tf_list(expr* dest,int list,int label);
void merge_tf_list(expr* left,expr* right,expr* dest,int list);
void backpatch(expr* patched,int patcher,int list_to_patch);
void insert_jump_list(int label);
void backpatch_jat(int label);
void backpatch_rat(int cur,int label);
void backpatch_jaf(int cur,int lab);
void backpatch_else(int cur,int label);

enum scopespace_t currScopeSpace(void);
unsigned currScopeOffset(void);
void incCurrScopeOffset(void);
void restoreCurScopeOffset(unsigned old_offset);
void enterScopeSpace(void);
void exitScopeSpace(void);