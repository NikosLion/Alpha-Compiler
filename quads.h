
#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

unsigned total;
unsigned int currQuad;

unsigned programVarOffset;
unsigned functionLocalOffset;
unsigned formalArgOffset;
unsigned scopeSpaceCounter;

enum iopcode{
  assign,   and,          if_less,      funcstart,
  add,      or,           if_greater,   funcend,
  sub,      not,          jump,         tablecreate,
  mul,      if_eq,        call,         tablegetelem,
  Div,      if_noteq,     param,        tablesetelem,
  mod,      if_lesseq,    Return,
  uminus,   if_geatereq,  getretval
};

enum expr_t{
  var_e,            arithexpr_e,    constnum_e,
  tableitem_e,      boolexpr_e,     constbool_e,
  programfunc_e,    assignexpr_e,   conststring_e,
  libraryfunc_e,    newtable_e,     nil_e
};

enum scopespace_t{
  programvar, functionlocal, formalarg
};

enum symbol_t{
  var_s, programfunc_s, libraryfunc_s
};

typedef struct symbol{
  enum symbol_t type;
  enum scopespace_t space;
  char* name;
  unsigned offset;
  unsigned scope;
  unsigned line;
}symbol;

typedef struct expr{
  enum expr_t type;
  symbol* sym;
  struct expr* index;
  double numConst;
  char* strConst;
  unsigned char boolCOnst;
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


void emit(enum iopcode op,expr* arg1,expr* arg2,expr* result,unsigned label,unsigned line);
void expand(void);

enum scopespace_t currScopeSpace(void);
unsigned currScopeOffset(void);
void incCurrScopeOffset(void);
void restoreCurScopeOffset(unsigned old_offset);
void enterScopeSpace(void);
void exitScopeSpace(void);
