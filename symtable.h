
enum SymbolType{
    global=1 , local=2 , formal=3, userfunc=4 , libfunc=5
};

enum scopespace_t{
  programvar, functionlocal, formalarg
};


typedef struct SymbolTableEntry{
    int isActive;
    char *name;
    int scope;
    int line;
    enum SymbolType type;
    enum scopespace_t space;
    unsigned offset;
    struct FuncArg *args;
    struct SymbolTableEntry *scope_list_next;
    struct SymbolTableEntry *scope_next;
    union value{
      char* stringValue;
      int intValue;
      double realValue;
      int boolean;    //true 1,false 0,null -1
    }value;
}SymbolTableEntry;


typedef struct FuncArg{
    int isActive;
    char *name;
    int scope;
    int line;
    enum SymbolType type;
    struct FuncArg *next;
}FuncArg;


void insert_funcArg(int scope,int line,char *name);
int lookup_funcArgs(int scope,char *name);
int delete_call_args(int scope,int call_args_counter);
int change_name(char *name,char *new_name,int scope);
int change_type(char *name);

void HideVar(int scope);
void insert_SymTable(char *name,int scope,int line,int enu);
void print_symTable();
void init_symTable();
int lookup_symTable(char *name,int scope,int type);         //search_cond 0 for same scope lookup,1 for global lookup
int lookup_symTable2(char *name,int scope,int type);
int isUserFunc(int scope);
int checkFuncName(char *name,int scope);
int look_lib_func(char *name);
