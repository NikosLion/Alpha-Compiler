#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"symtable.h"




struct SymbolTableEntry *ScopeListHead=NULL;
struct SymbolTableEntry *newEntry=NULL;
struct FuncArg *newArg=NULL;

//struct s_expr *fasi=NULL;
//struct s_expr *fasi2=NULL;


void init_symTable(){
    insert_SymTable("print",0,0,5);
    insert_SymTable("input",0,0,5);
    insert_SymTable("objectmemberkeys",0,0,5);
    insert_SymTable("objectcopy",0,0,5);
    insert_SymTable("totalarguments",0,0,5);
    insert_SymTable("argument",0,0,5);
    insert_SymTable("typeof",0,0,5);
    insert_SymTable("strtonum",0,0,5);
    insert_SymTable("sqrt",0,0,5);
    insert_SymTable("cos",0,0,5);
    insert_SymTable("sin",0,0,5);
}

/*Typwnei ta functions mazi me ta formals tous
  dhladh ta formals typwnontai sto scope tis sunarthshs
  pou anhkoun anti gia ena scope eswterikotera.Ta insert
  gia ta formals ginontai sto swsto scope.
*/
void print_symTable(){

    SymbolTableEntry *fasi=ScopeListHead;
    SymbolTableEntry *fasi2=ScopeListHead;
    printf("    \n");

    while(fasi!=NULL){
        printf("%s  %d  \n","Scope : ",fasi->scope);
        printf("    \n");
        fasi2=fasi;
        while(fasi2!=NULL){
            printf("Token:  %s |  Scope:  %d |  Line:  %d |  Type: %d\n",fasi2->name,fasi2->scope,fasi2->line,fasi2->type);
            fasi2=fasi2->scope_next;
        }
        printf("\n");
        fasi=fasi->scope_list_next;
    }
    printf("    \n");
}


void HideVar(int scope){
    SymbolTableEntry *dif_scope_temp=ScopeListHead;
    SymbolTableEntry *dif_scope_prev=ScopeListHead;
    SymbolTableEntry *same_scope_temp=ScopeListHead;

    while((dif_scope_temp!=NULL)&&(dif_scope_temp->scope<scope)){
        dif_scope_prev=dif_scope_temp;
        dif_scope_temp=dif_scope_temp->scope_list_next;
    }
    if((dif_scope_temp!=NULL) && (dif_scope_temp->scope==scope)){
      dif_scope_prev=dif_scope_temp;
    }
    else if(dif_scope_temp==NULL){
        return;
    }
    else {
        same_scope_temp=dif_scope_prev;
        while(same_scope_temp!=NULL){
            same_scope_temp->isActive=0;
            same_scope_temp=same_scope_temp->scope_next;
        }
    }
    return;
}




void insert_SymTable(char *name,int scope,int line,int enu){

    newEntry=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

    if (newEntry==NULL){
        printf("ERROR: OUT_OF_MEMORY\n");
        return;
    }

    newEntry->isActive=1;
    newEntry->name=strdup(name);
    newEntry->scope=scope;
    newEntry->line=line;
    newEntry->type=enu;

    if(ScopeListHead==NULL){
        ScopeListHead=newEntry;
        ScopeListHead->scope_list_next=NULL;
        ScopeListHead->scope_next=NULL;
    }
    else{
        SymbolTableEntry *temp=ScopeListHead;
        SymbolTableEntry *prev=temp;

        if(temp->scope == newEntry->scope){                                               //insert sthn lista tou idiou scope gia scope 0
                newEntry->scope_next=temp;
                newEntry->scope_list_next=temp->scope_list_next;
                temp=newEntry;
                ScopeListHead=temp;
        }
        else if(newEntry->scope > temp->scope){                                            //vriskoume se poio scope einai
            while((temp!=NULL) && (newEntry->scope > temp->scope)){
                prev=temp;
                temp=temp->scope_list_next;
            }
            if(temp==NULL){                                                             //create first node for this scope
                prev->scope_list_next=newEntry;
                newEntry->scope_list_next=NULL;
            }
            else if(temp->scope == newEntry->scope){                                  //insert sthn lista tou idiou scope
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


/*return 1 if found else return 0*/
int lookup_symTable(char *name,int scope,int type){

    SymbolTableEntry *find_scope=ScopeListHead;
    SymbolTableEntry *temp=ScopeListHead;

	//printf("111111111\n");
    while((find_scope!=NULL)&&(find_scope->scope<scope)){
        find_scope=find_scope->scope_list_next;
    }
    if(find_scope==NULL){
        return 0;
    }
    temp=find_scope;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){

            if(/*(temp->isActive==1) &&*/ (temp->type==type)){
                return 1;
            }
        }
        temp=temp->scope_next;
    }
    return 0;
}

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

/*Tsekarei an uparxei function sto scope pou tis dineis*/
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

/*Returns 1 if there is a colission between the argument "name"
  and a libfunc name, otherwise returns 0.
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



void insert_funcArg(int scope,int line,char *name){

    newArg=(struct FuncArg*)malloc(sizeof(struct FuncArg));
    SymbolTableEntry *temp=ScopeListHead;
    int t=0;

    while((temp!=NULL) && (temp->scope_list_next!=NULL) && (t==0)){
        if(temp->scope < scope){
            temp=temp->scope_list_next;
            t=1;
        }
    }

    newArg->isActive=1;
    newArg->name=strdup(name);
    newArg->scope=scope;
    newArg->line=line;
    newArg->type=3;
    newArg->next=NULL;

    if(temp->args==NULL){
        temp->args=newArg;
    }
    else{
        newArg->next=temp->args;
        temp->args=newArg;
    }
    return;
}

/*Returns -1 if "name" shadows a library function,
  1 if we find a formal argument with the same name
  and 0 if we don't find anything.
*/
int lookup_funcArgs(int scope,char *name){

    SymbolTableEntry *temp=ScopeListHead;
    FuncArg *func_temp=temp->args;

    while((temp!=NULL) && (temp->scope != scope)){
        temp=temp->scope_list_next;
    }

	//koitame mono to prwto stoixeio tis scope_list
	//giati mas endiaferoun mono ta formals ths synarthshs
	//pou eisixthe teleutaia (h insert ginete stin arxh tis listas).
    while(func_temp!=NULL){

        if(strcmp(name,func_temp->name)==0){
            return 1;
        }
        func_temp=func_temp->next;
    }
    return 0;							//den to vrhke, ara proxwraei h insert
}

int delete_call_args(int scope,int call_args_counter){
    SymbolTableEntry *temp=ScopeListHead;
    SymbolTableEntry *prev=NULL;
    SymbolTableEntry *temp_next=NULL;
    int i=0;
    int j=0;
    if(scope==0){
      while(i<call_args_counter){
        temp->scope_next->scope_list_next=temp->scope_list_next;
        ScopeListHead=temp->scope_next;
        temp->scope_next=NULL;
        temp=ScopeListHead;
        i++;
      }
      return 1;
    }
    else{
      while(j<scope){
        if(temp!=NULL){
          prev=temp;
          temp=temp->scope_list_next;
          j++;
        }
        else{
          printf("ERROR: at function delete_call_args\n");
          return 0;
        }
      }
      while(i<call_args_counter){
        if(temp!=NULL){
          if(temp->scope_next!=NULL){
            temp->scope_next->scope_list_next=temp->scope_list_next;
            temp->scope_list_next=NULL;
            prev->scope_list_next=temp->scope_next;
            temp->scope_next=NULL;
            temp=prev->scope_list_next;
          }
          else{
            prev->scope_list_next=temp->scope_list_next;
            temp=NULL;
          }
        }
        i++;
      }
      return 1;
    }
}

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