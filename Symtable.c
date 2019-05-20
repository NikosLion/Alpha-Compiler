#include <stdlib.h>
#include <string.h>
#include"Symtable.h"



struct SymbolTableEntry *ScopeListHead=NULL;
struct SymbolTableEntry *newEntry=NULL;
struct FuncArg *newArg=NULL;
char *t_name="_T";
int temp_var_counter=0;
char *f_name="_F";
int temp_func_counter=0;
char *return_name="_R";
int temp_return_counter=0;

//////////////////////////////////////////////////////////////
/*Arxikopoiei ton symbol table me table
  me ta library functions.
*/
void init_symTable(){
    insert_SymTable("print",0,0,5,0,programvar,0);
    insert_SymTable("input",0,0,5,0,programvar,0);
    insert_SymTable("objectmemberkeys",0,0,5,0,programvar,0);
    insert_SymTable("objectcopy",0,0,5,0,programvar,0);
    insert_SymTable("totalarguments",0,0,5,0,programvar,0);
    insert_SymTable("argument",0,0,5,0,programvar,0);
    insert_SymTable("typeof",0,0,5,0,programvar,0);
    insert_SymTable("strtonum",0,0,5,0,programvar,0);
    insert_SymTable("sqrt",0,0,5,0,programvar,0);
    insert_SymTable("cos",0,0,5,0,programvar,0);
    insert_SymTable("sin",0,0,5,0,programvar,0);
}

/////////////////////////////////////////////////////
/*Typwnei ta periexomena tou symbol table
  ana scope.
*/
void print_symTable(FILE* out){

    SymbolTableEntry *fasi=ScopeListHead;
    SymbolTableEntry *fasi2=ScopeListHead;
    struct FuncArg *temp_args;

    fprintf(out,"    \n");
    fprintf(out,"####################################################\n\n");
    while(fasi!=NULL){
        fprintf(out,"%s %d  <<<<-------\n","  ------->>>> Scope : ",fasi->scope);
        fprintf(out,"    \n");
        fasi2=fasi;
        while(fasi2!=NULL){
            fprintf(out,"Token:  %s |  Scope:  %d |  Type:  %d |  Offset:  %d |  Line %d\n",fasi2->name,fasi2->scope,fasi2->type,fasi2->offset,fasi2->line);
            fasi2=fasi2->scope_next;
        }
        fprintf(out,"\n");
        fasi=fasi->scope_list_next;
    }
    fprintf(out,"    \n");
}

//////////////////////////////////////////////////////////////////////////
/*Apenergopoiei ta symvola sto scope pou
  dinetai mesw tou isActive field.
*/
int HideVar(int scope){
    SymbolTableEntry *dif_scope_temp=ScopeListHead;
    SymbolTableEntry *dif_scope_prev=ScopeListHead;
    SymbolTableEntry *same_scope_temp=ScopeListHead;

    while((dif_scope_temp!=NULL)&&(dif_scope_temp->scope<scope)){
        dif_scope_prev=dif_scope_temp;
        dif_scope_temp=dif_scope_temp->scope_list_next;
    }
    if(dif_scope_temp==NULL){
        return -1;
    }
    else if(dif_scope_temp->scope==scope){
      dif_scope_prev=dif_scope_temp;
    }
    same_scope_temp=dif_scope_prev;
    while(same_scope_temp!=NULL){
        same_scope_temp->isActive=0;
        same_scope_temp=same_scope_temp->scope_next;
    }
    return 0;
}

/////////////////////////////////////////////////////////
/*Eisagwgh neou symvolou ston symbol table me
  parametrous to onoma, to scope, th grammh pou
  vrethike to symvolo kai ton typo tou.
*/
void insert_SymTable(char *name,int scope,int line,int enu,unsigned offset,int space,unsigned func_locs){

    newEntry=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

    if (newEntry==NULL){
        printf("ERROR: OUT_OF_MEMORY, check symtable\n");
        return;
    }
    newEntry->isActive=1;
    newEntry->name=strdup(name);
    newEntry->scope=scope;
    newEntry->line=line;
    newEntry->type=enu;
    newEntry->offset=offset;
    newEntry->space=space;
    newEntry->func_locals=func_locs;

    if(ScopeListHead==NULL){
        ScopeListHead=newEntry;
        ScopeListHead->scope_list_next=NULL;
        ScopeListHead->scope_next=NULL;
    }
    else{
        SymbolTableEntry *temp=ScopeListHead;
        SymbolTableEntry *prev=temp;

        if(temp->scope == newEntry->scope){                                     //insert sthn lista tou idiou scope gia scope 0
                newEntry->scope_next=temp;
                newEntry->scope_list_next=temp->scope_list_next;
                temp=newEntry;
                ScopeListHead=temp;
        }
        else if(newEntry->scope < temp->scope){
            newEntry->scope_list_next=ScopeListHead;
            ScopeListHead=newEntry;
        }
        else if(newEntry->scope > temp->scope){                                 //vriskoume se poio scope einai
            while((temp!=NULL) && (newEntry->scope > temp->scope)){
                prev=temp;
                temp=temp->scope_list_next;
            }
            if(temp==NULL){                                                     //create first node for this scope
                prev->scope_list_next=newEntry;
                newEntry->scope_list_next=NULL;
            }
            else if(temp->scope == newEntry->scope){                            //insert sthn lista tou idiou scope
                newEntry->scope_next=temp;
                newEntry->scope_list_next=temp->scope_list_next;
                temp=newEntry;
                prev->scope_list_next=temp;
            }
            else if(temp->scope > newEntry->scope){
                newEntry->scope_list_next=temp;
                prev->scope_list_next=newEntry;
            }
        }
    }
    return;
}

////////////////////////////////////////////////////////////
/*Lookup sto scope pou dinetai me vash to onoma
  kai ton typo tou symvolou pou psaxnoume.Epistrefei
  1 an vrethei to symvolo, 0 an oxi.
*/
int lookup_symTable(char *name,int scope,int type){

    SymbolTableEntry *find_scope=ScopeListHead;
    SymbolTableEntry *temp=ScopeListHead;

    while((find_scope!=NULL)&&(find_scope->scope<scope)){
        find_scope=find_scope->scope_list_next;
    }
    if(find_scope==NULL){
        return 0;
    }
    temp=find_scope;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){

            if(temp->type==type){
                return 1;
            }
        }
        temp=temp->scope_next;
    }
    return 0;
}

///////////////////////////////////////////////////////////////
/*Lookup sto scope pou dinetai gia active
  symvola.An vrethei to symvolo epistrefei to
  scope pou vrethike.An den uparxei to scope
  pou mas endiaferei epistrefei -2.An to symvolo
  den vrethei epistrefei -1.
*/
int lookup_symTable2(char *name,int scope,int type){

    SymbolTableEntry *find_scope=ScopeListHead;
    SymbolTableEntry *temp=ScopeListHead;

    int found_scope=-1;

    while((find_scope!=NULL)&&(find_scope->scope<scope)){
        find_scope=find_scope->scope_list_next;
    }
    if(find_scope==NULL){
        return -2;
    }
    temp=find_scope;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){
            if((temp->isActive==1) && (temp->type==type)){
              found_scope=temp->scope;
              return found_scope;
            }
        }
        temp=temp->scope_next;
    }
    return -2;
}

////////////////////////////////////////////////////////
/*Elegxei an to teleutaio symvolo pou egine eisagwgh
  sto scope pou dinetai einai synarthsh.
*/
int isUserFunc(int scope){
    SymbolTableEntry *temp=ScopeListHead;
    while((temp!=NULL)&&(temp->scope<scope)){
        temp=temp->scope_list_next;
    }
    if(temp!=NULL){
  		if(temp->type==4 && temp->isActive==1){
  			return 1;
  		}
  		else{
  			return 0;
  		}
    }
}

/////////////////////////////////////////////////////
/*Elegxei an to onoma pou dinetai sto
  sygkekrimeno scope antistoixei se onoma
  synarthshs.Ean nai epistrefei 1, diaforetika
  epistrefei 0.
*/
int checkFuncName(char *name,int scope){
    SymbolTableEntry *temp=ScopeListHead;
    SymbolTableEntry *side_temp=temp;

    int temp_scope=0;
    int found;
    while((temp!=NULL)&&(temp_scope<=scope)){
        side_temp=temp;
        while((side_temp!=NULL)&&(found==0)){
            found=lookup_symTable(name,temp_scope,4);
            if(found==1){
                return 1;
            }
            side_temp=side_temp->scope_next;
        }
        temp=temp->scope_list_next;
    }
    return 0;
}

////////////////////////////////////////////////////////
/*Epistrefei 1 ean to onoma pou dinetai antistoixei
  se onoma synarthshs vivliothikis, diaforetika
  epistrefei 0.
*/
int look_lib_func(char *name){
    int lib_name=0;
    if(strcmp(name,"print")==0){
      lib_name=1;
    }
    else if(strcmp(name,"input")==0){
      lib_name=1;
    }
    else if(strcmp(name,"objectmemberkeys")==0){
      lib_name=1;
    }
    else if(strcmp(name,"objectcopy")==0){
      lib_name=1;
    }
    else if(strcmp(name,"totalarguments")==0){
      lib_name=1;
    }
    else if(strcmp(name,"argument")==0){
      lib_name=1;
    }
    else if(strcmp(name,"typeof")==0){
      lib_name=1;
    }
    else if(strcmp(name,"strtonum")==0){
      lib_name=1;
    }
    else if(strcmp(name,"sqrt")==0){
      lib_name=1;
    }
    else if(strcmp(name,"cos")==0){
      lib_name=1;
    }
    else if(strcmp(name,"sin")==0){
      lib_name=1;
    }
    return lib_name;
}

/////////////////////////////////////////////////////////
/*Epistrefei -1 an to "name" sygkrouetai
  me kapoio apo ta library function,
  1 an vrethei formal argument me to idio onoma
  kai 0 an den vrethei tipota.
*/
int lookup_funcArgs(int scope,char *name){

    SymbolTableEntry *temp=ScopeListHead;
    FuncArg *func_temp;

    while((temp!=NULL) && (temp->scope != scope)){
        temp=temp->scope_list_next;
    }

    if(temp!=NULL){
      func_temp=temp->args;
      if(func_temp!=NULL){
        while(func_temp!=NULL){
          if(strcmp(name,func_temp->name)==0){
              return 1;
          }
          func_temp=func_temp->next;
        }
      }
    }
    return 0;							//den to vrhke, ara proxwraei h insert
}

////////////////////////////////////////////////////////////////
/*Allazei to onoma tou symvolou "name" se
  "new_name". Kaleitai otan to deksiotero
  expr se stoixeio indexedelem einai typou
  synarthshs, wste na mporoume na anaferthoume
  se auto mesw tou aristerou expr.
*/
int change_name(char *name,char *new_name,int scope){
  SymbolTableEntry *find_scope=ScopeListHead;
  SymbolTableEntry *temp=ScopeListHead;

  while((find_scope!=NULL)&&(find_scope->scope<scope)){
    find_scope=find_scope->scope_list_next;
  }
  temp=find_scope;
  while((find_scope!=NULL) && (!strcmp(find_scope->name,name))){
    temp=find_scope;
    find_scope=find_scope->scope_next;
  }
  if(temp!=NULL){
    char *res = malloc(strlen(new_name));
    strcpy(res,new_name);
    temp->name=res;
  }
  return 0;
}

/////////////////////////////////////////////////////////
/*Allazei ton typo tou symvolou "name" se
  "userfunc". Kaleitai otan to deksio melos
  enos assignexpr einai typou synarthshs wste
  na epitrapoun energeies "call" sto aristero
  melos se epomenes entoles ston kwdika tou
  xristi.
*/
int change_type(char *name){
  SymbolTableEntry *find_scope=ScopeListHead;
  SymbolTableEntry *temp=ScopeListHead;

  while(find_scope!=NULL){
    temp=find_scope;
    while(temp!=NULL){
      if(!strcmp(temp->name,name)){
        temp->type=4;
        return 0;
      }
      temp=temp->scope_next;
    }
    find_scope=find_scope->scope_list_next;
  }
  return 1;
}

///////////////////////////////////////////////////////
//concatanate 2 strings and return char*
char* concat(const char *s1, const char *s2){
  char *result = malloc(strlen(s1)+strlen(s2)+1);
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

//////////////////////////////////////////////////
//gives temp var name
char *temp_name() {
  char s[20];
  char *yo;
  sprintf(s,"%d",temp_var_counter);
  yo=concat(t_name,s);
  temp_var_counter++;
  return yo;
}

/////////////////////////////////////////////////
//gives func name
char *temp_name_func() {
  char s[20];
  char *yo;
  sprintf(s,"%d",temp_func_counter);
  yo=concat(f_name,s);
  temp_func_counter++;
  return yo;
}

/////////////////////////////////////////////////
char* temp_name_return(){
  char s[20];
  char *yo;
  sprintf(s,"%d",temp_return_counter);
  yo=concat(return_name,s);
  temp_return_counter++;
  return yo;
}
