%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Table_queue.h"
#include "Offset_stack.h"
#include "Function_stack.h"


int func_name = 0;
unsigned int scope = 0;
int inside_function=0;
int inside_loop=0;
int lvalue_flag=0;
int const_flag=0;
int func_flag=0;
int call_args_counter=0;
int lab=0;
int elist_counter=0;
struct expr* index_head;
struct expr* elist_head;



int yylex(void);
void yyerror(const char *s);

%}

%defines
%error-verbose
%start program

%union {
  char* stringValue;
  int intValue;
  double realValue;
  struct expr *exprNode;
  struct FuncArg *argument_t;
};

%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL ASSIGN PLUS MINUS MUL DIV MODULO EQUALS NOT_EQUALS INCR DECR GREATER LESS GR_EQUAL LESS_EQUAL L_CURLY R_CURLY L_BRACKET R_BRACKET L_PARENTHESIS R_PARENTHESIS SEMICOLON COMMA COLON DOUBLE_COLON DOT DOUBLE_DOT
%token<intValue> INTEGER
%token<realValue> REAL
%token<stringValue> IDENTIFIER STRINGLITERAL


%type<exprNode> lvalue funcdef const assignexpr expr term primary stmt booleanop relativeop arithmeticop call ifprefix ifstmt whilestmt forstmt elist member objectdef indexed indexedelem
%type<argument_t> idlist
%type<stringValue>   returnstmt block
%type<stringValue>  normcall methodcall callsuffix

%right		  ASSIGN
%left     	OR
%left     	AND
%nonassoc	  EQUALS NOT_EQUALS
%nonassoc	  GREATER GR_EQUAL LESS LESS_EQUAL
%left       PLUS MINUS
%left		    MUL DIV MODULO
%right      NOT INCR DECR UMINUS
%left       DOT DOUBLE_DOT
%left		    L_BRACKET R_BRACKET
%left		    L_PARENTHESIS R_PARENTHESIS


%%


program:	program stmt {
            fprintf(GOUT,"program: program stmt\n");
          }

	     |   {
              fprintf(GOUT,"program: \n");
            }
	     ;

stmt:	expr SEMICOLON  {
        fprintf(GOUT,"stmt: expr ;\n");

        if($1!=NULL){
          if($1->type==boolexpr_e){
            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));

            struct SymbolTableEntry *sym;
            sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
            temp->sym=sym;

            temp->sym->offset=currScopeOffset();
            temp->sym->space=currScopeSpace();
            temp->sym->name=temp_name();
            temp->type=var_e;
            incCurrScopeOffset();

            if(scope==0){
              insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
            }
            else{
              insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
            }

            struct expr *temp_true;
            temp_true=(struct expr*)malloc(sizeof(struct expr));
            temp_true->type=constbool_e;
            temp_true->value.boolean=1;
            temp_true->int_real=-2;

            struct expr *temp_false;
            temp_false=(struct expr*)malloc(sizeof(struct expr));
            temp_false->type=constbool_e;
            temp_false->value.boolean=0;
            temp_false->int_real=-2;


            emit(assign,temp_true,NULL,temp,0,yylineno);

            backpatch($1,currQuad-1,1);

            emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
            emit(assign,temp_false,NULL,temp,0,yylineno);

            backpatch($1,currQuad-1,0);
          }
        }
      }

    | ifprefix  {
       fprintf(GOUT,"stmt: ifprefix\n");
       struct expr *temp_true;
       temp_true=(struct expr*)malloc(sizeof(struct expr));
       temp_true->type=constbool_e;
       temp_true->value.boolean=1;
       temp_true->int_real=-2;

       struct expr *temp_false;
       temp_false=(struct expr*)malloc(sizeof(struct expr));
       temp_false->type=constbool_e;
       temp_false->value.boolean=0;
       temp_false->int_real=-2;

       struct expr *temp;
       temp=(struct expr*)malloc(sizeof(struct expr));
       struct SymbolTableEntry *sym;
       sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
       temp->sym=sym;
       temp->sym->offset=currScopeOffset();
       temp->sym->space=currScopeSpace();
       temp->sym->name=temp_name();
       temp->type=var_e;
       incCurrScopeOffset();

       if(scope==0){
         insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
       }
       else{
         insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
       }

       emit(jump,NULL,NULL,NULL,0,yylineno);
       emit(assign,temp_true,NULL,temp,0,yylineno);

       backpatch($1,currQuad-1,1);

       emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
       emit(assign,temp_false,NULL,temp,0,yylineno);

       backpatch($1,currQuad-1,0);

       emit(if_eq,temp,temp_true,NULL,0,yylineno);

       backpatch_rat(currQuad,$1->true_list->label);
       backpatch_jat(currQuad);

       lab=$1->true_list->label;
       $$=$1;
     }


    | ifstmt  {
        fprintf(GOUT,"stmt: ifstmt\n");

        struct expr *temp_true;
        temp_true=(struct expr*)malloc(sizeof(struct expr));
        temp_true->type=constbool_e;
        temp_true->value.boolean=1;
        temp_true->int_real=-2;

        struct expr *temp_false;
        temp_false=(struct expr*)malloc(sizeof(struct expr));
        temp_false->type=constbool_e;
        temp_false->value.boolean=0;
        temp_false->int_real=-2;

        struct expr *temp;
        temp=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        temp->sym=sym;
        temp->sym->offset=currScopeOffset();
        temp->sym->space=currScopeSpace();
        temp->sym->name=temp_name();
        temp->type=var_e;
        incCurrScopeOffset();

        if(scope==0){
          insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
        }
        else{
          insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
        }

        emit(jump,NULL,NULL,NULL,0,yylineno);
        emit(assign,temp_true,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,1);

        emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
        emit(assign,temp_false,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,0);

        emit(if_eq,temp,temp_true,NULL,0,yylineno);

        backpatch_rat(currQuad,$1->true_list->label);
        backpatch_jat(currQuad+1);

        emit(jump,NULL,NULL,NULL,0,yylineno);

        backpatch_jaf(currQuad-1,lab);
        backpatch_else(currQuad-1,($1->value.intValue)+1);
        lab=$1->true_list->label;
        $$=$1;
      }

    | whilestmt {
        fprintf(GOUT,"stmt: whilestmt\n");

        struct expr *temp_true;
        temp_true=(struct expr*)malloc(sizeof(struct expr));
        temp_true->type=constbool_e;
        temp_true->value.boolean=1;
        temp_true->int_real=-2;

        struct expr *temp_false;
        temp_false=(struct expr*)malloc(sizeof(struct expr));
        temp_false->type=constbool_e;
        temp_false->value.boolean=0;
        temp_false->int_real=-2;

        struct expr *temp;
        temp=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        temp->sym=sym;
        temp->sym->offset=currScopeOffset();
        temp->sym->space=currScopeSpace();
        temp->sym->name=temp_name();
        temp->type=var_e;
        incCurrScopeOffset();

        if(scope==0){
          insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
        }
        else{
          insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
        }

        emit(jump,NULL,NULL,NULL,$1->true_list->label,yylineno);
        emit(assign,temp_true,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,1);

        emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
        emit(assign,temp_false,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,0);

        emit(if_eq,temp,temp_true,NULL,($1->false_list->label)+1,yylineno);

        if(break_head!=NULL){
          backpatch_break(currQuad);
        }
        if(continue_head!=NULL){
          backpatch_continue($1->true_list->label);
        }

        $$=$1;
      }

    | forstmt {
        fprintf(GOUT,"stmt: forstmt\n");

        struct expr *temp_true;
        temp_true=(struct expr*)malloc(sizeof(struct expr));
        temp_true->type=constbool_e;
        temp_true->value.boolean=1;
        temp_true->int_real=-2;

        struct expr *temp_false;
        temp_false=(struct expr*)malloc(sizeof(struct expr));
        temp_false->type=constbool_e;
        temp_false->value.boolean=0;
        temp_false->int_real=-2;

        struct expr *temp;
        temp=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        temp->sym=sym;
        temp->sym->offset=currScopeOffset();
        temp->sym->space=currScopeSpace();
        temp->sym->name=temp_name();
        temp->type=var_e;
        incCurrScopeOffset();

        if(scope==0){
          insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
        }
        else{
          insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
        }

        emit(jump,NULL,NULL,NULL,$1->true_list->label,yylineno);
        emit(assign,temp_true,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,1);

        emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
        emit(assign,temp_false,NULL,temp,0,yylineno);

        backpatch($1,currQuad-1,0);

        emit(if_eq,temp,temp_true,NULL,($1->false_list->label)+1,yylineno);

        if(break_head!=NULL){
          backpatch_break(currQuad);
        }
        if(continue_head!=NULL){
          backpatch_continue($1->true_list->label);
        }

        $$=$1;
      }
    | returnstmt  {
        fprintf(GOUT,"stmt: returnstmt\n");
      }
    | BREAK SEMICOLON {
        fprintf(GOUT,"statement: break ;\n");
        if(inside_loop==0){
        	fprintf(GOUT,"Error at line %d: Break statement not inside a loop.\n", yylineno);
          exit(0);
        }
        insert_break_list(currQuad);
        emit(jump,NULL,NULL,NULL,0,yylineno);
	    }
    | CONTINUE SEMICOLON  {
    		fprintf(GOUT,"statement: continue ;\n");
    		if(inside_loop==0){
    			fprintf(GOUT,"Error at line %d: Continue statement not inside a loop.\n", yylineno);
          exit(0);
    		}
        insert_continue_list(currQuad);
        emit(jump,NULL,NULL,NULL,0,yylineno);
    	}
    | block {fprintf(GOUT,"stmt: block\n");}
    | funcdef {fprintf(GOUT,"stmt: funcdef\n");}
    | SEMICOLON {fprintf(GOUT,"stmt: SEMICOLON\n");}
	  ;

expr:	assignexpr  {fprintf(GOUT,"expr: assignexpr\n"); $$=$1;}
    | arithmeticop  {fprintf(GOUT,"expr: arithmeticop\n"); $$=$1;}
    | booleanop {fprintf(GOUT,"expr: booleanop\n"); $$=$1;}
    | relativeop  {fprintf(GOUT,"expr: relativeop\n"); $$=$1;}
    | term {
        fprintf(GOUT,"expr: term\n");
        if(lvalue_flag==1){
          lvalue_flag=0;
        }
        else if(const_flag==1){
          const_flag=0;
        }
        else if(func_flag==1){
          func_flag=0;
        }
        $$=$1;
      }
    ;

arithmeticop: expr PLUS expr  {
                fprintf(GOUT,"op: expr + expr\n");
                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                temp->sym=sym;
                temp->sym->offset=currScopeOffset();
                temp->sym->space=currScopeSpace();
                temp->sym->name=temp_name();
                temp->type=var_e;
                incCurrScopeOffset();
                if(scope==0){
                  insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                }
                else{
                  insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                }
                emit(add,$1,$3,temp,0,yylineno);
                $$=temp;
              }
  			   |  expr MINUS expr  {
                fprintf(GOUT,"op: expr - expr\n");
                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                temp->sym=sym;
                temp->sym->offset=currScopeOffset();
                temp->sym->space=currScopeSpace();
                temp->sym->name=temp_name();
                temp->type=var_e;
                incCurrScopeOffset();
                if(scope==0){
                  insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                }
                else{
                  insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                }
                emit(sub,$1,$3,temp,0,yylineno);
                $$=temp;
              }
  			   |	expr MUL expr {
                fprintf(GOUT,"op: expr * expr\n");
                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                temp->sym=sym;
                temp->sym->offset=currScopeOffset();
                temp->sym->space=currScopeSpace();
                temp->sym->name=temp_name();
                temp->type=var_e;
                incCurrScopeOffset();
                if(scope==0){
                  insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                }
                else{
                  insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                }
                emit(mul,$1,$3,temp,0,yylineno);
                $$=temp;
              }
			     | 	expr DIV expr  {
                fprintf(GOUT,"op: expr / expr\n");
                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                temp->sym=sym;
                temp->sym->offset=currScopeOffset();
                temp->sym->space=currScopeSpace();
                temp->sym->name=temp_name();
                temp->type=var_e;
                incCurrScopeOffset();
                if(scope==0){
                  insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                }
                else{
                  insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                }
                emit(Div,$1,$3,temp,0,yylineno);
                $$=temp;
              }
			     | 	expr MODULO expr {
                fprintf(GOUT,"op: expr Modulo expr\n");
                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                temp->sym=sym;
                temp->sym->offset=currScopeOffset();
                temp->sym->space=currScopeSpace();
                temp->sym->name=temp_name();
                temp->type=var_e;
                incCurrScopeOffset();
                if(scope==0){
                  insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                }
                else{
                  insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                }
                emit(mod,$1,$3,temp,0,yylineno);
                $$=temp;
              }
			     ;

relativeop:		expr GREATER expr  {
                fprintf(GOUT,"op: expr > expr\n");

                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                temp->type=boolexpr_e;

                emit(if_greater,$1,$3,NULL,0,yylineno);
                emit(jump,NULL,NULL,NULL,0,yylineno);
                insert_tf_list(temp,1,currQuad-2);
                insert_tf_list(temp,0,currQuad-1);
                $$=temp;
              }
		  |		expr GR_EQUAL expr {
            fprintf(GOUT,"op: expr >= expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;

            emit(if_greatereq,$1,$3,NULL,0,yylineno);
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_tf_list(temp,1,currQuad-2);
            insert_tf_list(temp,0,currQuad-1);
            $$=temp;
          }
		  |		expr LESS expr {
            fprintf(GOUT,"op: expr < expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;

            emit(if_less,$1,$3,NULL,0,yylineno);
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_tf_list(temp,1,currQuad-2);
            insert_tf_list(temp,0,currQuad-1);
            $$=temp;
          }
		  |		expr LESS_EQUAL expr {
            fprintf(GOUT,"op: expr <= expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;

            emit(if_lesseq,$1,$3,NULL,0,yylineno);
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_tf_list(temp,1,currQuad-2);
            insert_tf_list(temp,0,currQuad-1);
            $$=temp;
          }
		  |		expr EQUALS expr {
            fprintf(GOUT,"op: expr == expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;

            emit(if_eq,$1,$3,NULL,0,yylineno);
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_tf_list(temp,1,currQuad-2);
            insert_tf_list(temp,0,currQuad-1);
            $$=temp;
          }
		  |		expr NOT_EQUALS expr {
            fprintf(GOUT,"op: expr != expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;

            emit(if_noteq,$1,$3,NULL,0,yylineno);
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_tf_list(temp,1,currQuad-2);
            insert_tf_list(temp,0,currQuad-1);
            $$=temp;
          }
		  ;

booleanop:		expr AND expr {
                fprintf(GOUT,"op: expr && expr\n");

                struct expr *temp;
                temp=(struct expr*)malloc(sizeof(struct expr));
                temp->type=boolexpr_e;
                temp->int_real=-6;

                struct expr *temp_true;
                temp_true=(struct expr*)malloc(sizeof(struct expr));
                temp_true->type=constbool_e;
                temp_true->value.boolean=1;
                temp_true->int_real=-2;

                struct expr *temp_false;
                temp_false=(struct expr*)malloc(sizeof(struct expr));
                temp_false->type=constbool_e;
                temp_false->value.boolean=0;
                temp_false->int_real=-2;

                if(($1->type==var_e) || ($1->type!=boolexpr_e)){
                  emit(if_eq,$1,temp_true,NULL,0,yylineno);
                  emit(jump,NULL,NULL,NULL,0,yylineno);
                  insert_tf_list($1,1,currQuad-2);
                  insert_tf_list($1,0,currQuad-1);
                }
                if(($3->type==var_e) || ($3->type!=boolexpr_e)){
                  emit(if_eq,$3,temp_true,NULL,0,yylineno);
                  emit(jump,NULL,NULL,NULL,0,yylineno);
                  insert_tf_list($3,1,currQuad-2);
                  insert_tf_list($3,0,currQuad-1);
                }

                if((($1->type==var_e) || ($1->type!=boolexpr_e)) && $3->type==boolexpr_e){
                  backpatch($3,$1->true_list->label,1);
                  temp->true_list=$1->true_list;
                  merge_tf_list($1,$3,temp,0);
                }
                else{
                  backpatch($1,$3->true_list->label,1);
                  temp->true_list=$3->true_list;
                  merge_tf_list($1,$3,temp,0);
                }
                $$=temp;
              }
		 |		expr OR expr {
            fprintf(GOUT,"op: expr || expr\n");

            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            temp->type=boolexpr_e;
            temp->int_real=-5;

            struct expr *temp_true;
            temp_true=(struct expr*)malloc(sizeof(struct expr));
            temp_true->type=constbool_e;
            temp_true->value.boolean=1;
            temp_true->int_real=-2;

            if(($1->type==var_e) || ($1->type!=boolexpr_e)){
              emit(if_eq,$1,temp_true,NULL,0,yylineno);
              emit(jump,NULL,NULL,NULL,0,yylineno);
              insert_tf_list($1,1,currQuad-2);
              insert_tf_list($1,0,currQuad-1);
            }
            if(($3->type==var_e) || ($3->type!=boolexpr_e)){
              emit(if_eq,$3,temp_true,NULL,0,yylineno);
              emit(jump,NULL,NULL,NULL,0,yylineno);
              insert_tf_list($3,1,currQuad-2);
              insert_tf_list($3,0,currQuad-1);
            }

            if((($1->type==var_e) || ($1->type!=boolexpr_e)) && $3->type==boolexpr_e){
              backpatch($3,$1->true_list->label,0);
              merge_tf_list($1,$3,temp,1);
              temp->false_list=$1->false_list;
            }
            else{
              //backpatch $1->false_list sto label tou quad tou $3
              backpatch($1,$3->true_list->label,0);
              merge_tf_list($1,$3,temp,1);
              temp->false_list=$3->false_list;
            }
            $$=temp;
          }
		 ;

term:		L_PARENTHESIS expr R_PARENTHESIS 	{
          fprintf(GOUT,"term: ( expr )\n");
          $$=$2;
        }
	|		MINUS expr %prec UMINUS	{
        fprintf(GOUT,"term: -expr\n");
        struct expr *temp_const;
        temp_const=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        temp_const->sym=sym;
        temp_const->type=var_e;
        temp_const->sym->name=temp_name();

        incCurrScopeOffset();
        if(scope==0){
          insert_SymTable(temp_const->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
        }
        else{
          insert_SymTable(temp_const->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
        }

        emit(uminus,$2,NULL,temp_const,0,yylineno);
        $$=temp_const;
      }
	|		NOT expr {
        fprintf(GOUT,"term: ! expr\n");
        struct expr *temp_not;
        temp_not=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        temp_not->sym=sym;
        temp_not->type=boolexpr_e;
        temp_not->sym->name=temp_name();

        incCurrScopeOffset();
        if(scope==0){
          insert_SymTable(temp_not->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
        }
        else{
          insert_SymTable(temp_not->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
        }

        struct expr *temp_true;
        temp_true=(struct expr*)malloc(sizeof(struct expr));
        temp_true->type=constbool_e;
        temp_true->value.boolean=1;
        temp_true->int_real=-2;

        if(($2->type==var_e) || ($2->type!=boolexpr_e)){
          emit(if_eq,$2,temp_true,NULL,-1,yylineno);
          emit(jump,NULL,NULL,NULL,-1,yylineno);
          insert_tf_list(temp_not,1,currQuad-1);
          insert_tf_list(temp_not,0,currQuad-2);
        }
        else{
          tf_node* temp_list;
          temp_list=$2->true_list;
          $2->true_list=$2->false_list;
          $2->false_list=temp_list;
          temp_not=$2;
        }
        $$=temp_not;
      }
	|		INCR lvalue{
        fprintf(GOUT,"term: ++lvalue\n");
        if($2->type==programfunc_e || $2->type==libraryfunc_e){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$2->sym->name);
          exit(0);
        }
        struct expr *temp_const;
        temp_const=(struct expr*)malloc(sizeof(struct expr));
        temp_const->type=constnum_e;
        temp_const->value.intValue=1;
        temp_const->int_real=1;
        emit(add,temp_const,$2,$2,0,yylineno);
        $$=$2;
      }
	|		lvalue INCR	{
        fprintf(GOUT,"term: lvalue++\n");
        if($1->type==programfunc_e || $1->type==libraryfunc_e){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$1->sym->name);
          exit(0);
        }
        struct expr *temp_const;
        temp_const=(struct expr*)malloc(sizeof(struct expr));
        temp_const->type=constnum_e;
        temp_const->value.intValue=1;
        temp_const->int_real=1;
        emit(add,$1,temp_const,$1,0,yylineno);
        $$=$1;
      }
	|		DECR lvalue{
        fprintf(GOUT,"term: --lvalue\n");
        if($2->type==programfunc_e || $2->type==libraryfunc_e){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$2->sym->name);
          exit(0);
        }
        struct expr *temp_const;
        temp_const=(struct expr*)malloc(sizeof(struct expr));
        temp_const->type=constnum_e;
        temp_const->value.intValue=1;
        temp_const->int_real=1;
        emit(sub,temp_const,$2,$2,0,yylineno);
        $$=$2;
      }
	|		lvalue DECR{
        fprintf(GOUT,"term: lvalue--\n");
        if($1->type==programfunc_e || $1->type==libraryfunc_e){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$1->sym->name);
          exit(0);
        }
        struct expr *temp_const;
        temp_const=(struct expr*)malloc(sizeof(struct expr));
        temp_const->type=constnum_e;
        temp_const->value.intValue=1;
        temp_const->int_real=1;
        emit(sub,$1,temp_const,$1,0,yylineno);
        $$=$1;
      }
	|		primary{
        fprintf(GOUT,"term: primary\n");
        if(lvalue_flag==1){
          $$=$1;
        }
        else if(const_flag==1){
          $$=$1;
        }
        else if(func_flag==1){
          $$=$1;
        }
      }
	;

assignexpr:	lvalue ASSIGN expr{
              fprintf(GOUT,"assignexpr: lvalue = expr\n");

              if($1->type==programfunc_e || $1->type==libraryfunc_e){
                fprintf(GOUT,"Error at line %d: Cannot assign to Funtion %s \n",yylineno,$1->sym->name);
                exit(0);
              }
              if($3->type==programfunc_e || $3->type==libraryfunc_e){
                change_type($1->sym->name);
                $1->type=$3->type;                  //isws na prepei na mpei e3w apo thn if
              }

              if($1->index==NULL){

                if($3->type==boolexpr_e){

                  struct expr *temp_true;
                  temp_true=(struct expr*)malloc(sizeof(struct expr));
                  temp_true->type=constbool_e;
                  temp_true->value.boolean=1;
                  temp_true->int_real=-2;

                  struct expr *temp_false;
                  temp_false=(struct expr*)malloc(sizeof(struct expr));
                  temp_false->type=constbool_e;
                  temp_false->value.boolean=0;
                  temp_false->int_real=-2;

                  struct expr *temp;
                  temp=(struct expr*)malloc(sizeof(struct expr));
                  struct SymbolTableEntry *sym;
                  sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                  temp->sym=sym;
                  temp->sym->offset=currScopeOffset();
                  temp->sym->space=currScopeSpace();
                  temp->sym->name=temp_name();
                  temp->type=var_e;

                  incCurrScopeOffset();

                  if(scope==0){
                    insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                  }
                  else{
                    insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                  }

                  emit(assign,temp_true,NULL,temp,0,yylineno);

                  backpatch($3,currQuad-1,1);

                  emit(jump,NULL,NULL,NULL,currQuad+2,yylineno);
                  emit(assign,temp_false,NULL,temp,0,yylineno);

                  backpatch($3,currQuad-1,0);

                  emit(assign,temp,NULL,$1,0,yylineno);


                  struct expr *temp2;
                  temp2=(struct expr*)malloc(sizeof(struct expr));
                  struct SymbolTableEntry *sym2;
                  sym2=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                  temp2->sym=sym2;
                  temp2->sym->offset=currScopeOffset();
                  temp2->sym->space=currScopeSpace();
                  temp2->sym->name=temp_name();
                  temp2->type=var_e;

                  emit(assign,$1,NULL,temp2,0,yylineno);
                  $$=temp2;
                }
                else{
                  struct expr *temp;
                  temp=(struct expr*)malloc(sizeof(struct expr));
                  struct SymbolTableEntry *sym;
                  sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                  temp->sym=sym;
                  temp->sym->name=temp_name();
                  temp->type=var_e;

                  incCurrScopeOffset();
                  if(scope==0){
                    insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
                  }
                  else{
                    insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
                  }

                  emit(assign,$3,NULL,$1,0,yylineno);
                  emit(assign,$1,NULL,temp,0,yylineno);

                  $$=temp;
                }
              }
              else{
                emit(tablesetelem,$3,NULL,$1,0,yylineno);
              }
              $$=$1;
          }
		  ;

primary:	lvalue{
    				    fprintf(GOUT,"primary: lvalue\n");
    				    lvalue_flag=1;
    				    $$=$1;
			    }
	   |	  call {
            fprintf(GOUT,"primary: call\n");
          }
	   |	  objectdef {
            fprintf(GOUT,"primary: objectdef\n");
            elist_counter=0;
            index_head=NULL;
            elist_head=NULL;
          }

	   |	  L_PARENTHESIS funcdef R_PARENTHESIS {
            fprintf(GOUT,"primary: ( funcdef )\n");
            func_flag=1;
            $$=$2;
          }
	   |	  const	{
                fprintf(GOUT,"primary: const\n");
                const_flag=1;
                $$=$1;
          }
	   ;

lvalue:		IDENTIFIER {

      fprintf(GOUT,"lvalue: IDENTIFIER\n");
			int tmp_scope = scope;
			int found = 0;
			int type=0;
			int found_scope=-1;

      struct expr *temp_maloc;
      temp_maloc=(struct expr*)malloc(sizeof(struct expr));

      struct SymbolTableEntry *sym;
      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

      temp_maloc->sym=sym;

			//arxika check an einai libfunc
			if(look_lib_func($1)){
				temp_maloc->sym->name=$1;
				temp_maloc->type=libraryfunc_e;;
				$$=temp_maloc;
				break;
			}
			//psaxnoume ta scope apo mesa pros ta eksw
			while(tmp_scope >= 0){
				//periptwseis pou vriskoume kati
				if((found_scope=lookup_symTable2($1,tmp_scope,1))!=-2){
					type=1;
					temp_maloc->sym->name=$1;
					temp_maloc->type=var_e;
          temp_maloc->sym->type=type;
					$$=temp_maloc;
    			break;
    		}
    		else if((found_scope=lookup_symTable2($1,tmp_scope,2))!=-2){
					type=2;
					int tmp_scope2=scope;
					while(tmp_scope2>=tmp_scope){
						if(isUserFunc(tmp_scope2)){
							fprintf(GOUT,"Error at line %d: Cannot access local identifier %s \n",yylineno,$1);
							exit(0);
						}
						tmp_scope2--;
					}
					temp_maloc->sym->name=$1;
          temp_maloc->sym->type=type;
          temp_maloc->type=var_e;
					$$=temp_maloc;
    			break;
    		}
  			else if((found_scope=lookup_symTable2($1,tmp_scope,3))!=-2){
  				type=3;
  				if(scope==found_scope){
					temp_maloc->sym->name=$1;
					temp_maloc->sym->type=type;
          temp_maloc->type=var_e;
					$$=temp_maloc;
  				break;
  				}
  				else{
  					fprintf(GOUT,"Error at line %d: Cannot access formal identifier %s \n",yylineno,$1);
  					exit(0);
  				}
    		}
  			else if((found_scope=lookup_symTable2($1,tmp_scope,4))!=-2){
  				type=4;
				  temp_maloc->sym->name=$1;
				  temp_maloc->sym->type=type;
          temp_maloc->type=programfunc_e;
				  $$=temp_maloc;
  				break;
  			}
				tmp_scope--;
			}

			//an dn vrethei tipota kanoume insert sto trexon scope
			if(type==0){
    			if(scope == 0){
            incCurrScopeOffset();
            temp_maloc->sym->offset=currScopeOffset();
            temp_maloc->sym->space=currScopeSpace();
            temp_maloc->sym->name=$1;
            temp_maloc->sym->type=1;
            temp_maloc->type=var_e;
    				insert_SymTable($1,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
					  $$=temp_maloc;

    			}
    			else{
            incCurrScopeOffset();
            temp_maloc->sym->offset=currScopeOffset();
            temp_maloc->sym->space=currScopeSpace();
            temp_maloc->sym->name=$1;
					  temp_maloc->sym->type=2;
            temp_maloc->type=var_e;
    				insert_SymTable($1,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
					  $$=temp_maloc;
    			}
    		}
        reset_queue();
	  }
	  |  LOCAL IDENTIFIER  {

			fprintf(GOUT,"lvalue: local IDENTIFIER\n");

			struct expr *temp_maloc;
			temp_maloc=(struct expr*)malloc(sizeof(struct expr));

      struct SymbolTableEntry *sym;
      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

      temp_maloc->sym=sym;

			if(look_lib_func($2)){
				fprintf(GOUT,"Error at line %d: Local identifier trying to shadow library function: %s\n", yylineno,$2);
				exit(0);
			}
			//psaxnoume sto trexon scope
			if(lookup_symTable2($2,scope,2)!=-2){
				temp_maloc->sym->name=$2;
				temp_maloc->sym->type=2;
        temp_maloc->type=var_e;
				$$=temp_maloc;
			}
			else if(lookup_symTable2($2,scope,4)!=-2){
				temp_maloc->sym->name=$2;
				temp_maloc->sym->type=4;
        temp_maloc->type=programfunc_e;
				$$=temp_maloc;
			}
			//kanoume eisagwgh ston ST agnowntas to
			//keyword "local" an eimaste se scope 0
			else{
				if(scope == 0){
				  temp_maloc->sym->name=$2;
					temp_maloc->sym->type=1;
          temp_maloc->type=var_e;

          incCurrScopeOffset();
          temp_maloc->sym->offset=currScopeOffset();
          temp_maloc->sym->space=currScopeSpace();

          $$=temp_maloc;
					insert_SymTable($2,0,yylineno,1,currScopeOffset(),currScopeSpace(),0);

				}
				else{
					temp_maloc->sym->name=$2;
					temp_maloc->sym->type=2;
          temp_maloc->type=var_e;

          incCurrScopeOffset();
          temp_maloc->sym->offset=currScopeOffset();
          temp_maloc->sym->space=currScopeSpace();
					$$=temp_maloc;
					insert_SymTable($2,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);

				}
			}
      reset_queue();
	  }

	  |  DOUBLE_COLON IDENTIFIER {
			fprintf(GOUT,"lvalue: ::IDENTIFIER\n");

      reset_queue();

			struct expr *temp_maloc;
			temp_maloc=(struct expr*)malloc(sizeof(struct expr));

      struct SymbolTableEntry *sym;
      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

      temp_maloc->sym=sym;

			if(lookup_symTable($2,0,1)){
			  temp_maloc->sym->name=$2;
			  temp_maloc->sym->type=1;
        temp_maloc->type=var_e;
				$$=temp_maloc;
				break;
			}
			else if(lookup_symTable($2,0,4)){
				temp_maloc->sym->name=$2;
				temp_maloc->sym->type=4;
        temp_maloc->type=programfunc_e;
				$$=temp_maloc;
				break;
			}
			else{
				fprintf(GOUT,"Error at line %d: Invalid reference.No global identifier: %s\n", yylineno,$2);
				exit(0);
			}
	  }

	  |		member  {
        fprintf(GOUT,"lvalue: member\n");
        $$=$1;
    }
      ;

member:		lvalue DOT IDENTIFIER	{
          fprintf(GOUT,"member: lvalue.IDENTIFIER\n");

          struct expr *temp;
          temp=(struct expr*)malloc(sizeof(struct expr));
          struct expr *index;
          index=(struct expr*)malloc(sizeof(struct expr));
          struct SymbolTableEntry *sym;
          sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

          temp->index=index;
          temp->sym=sym;
          temp->type=var_e;
          temp->sym->offset=currScopeOffset();
          temp->sym->space=currScopeSpace();
          temp->sym->name=temp_name();
          incCurrScopeOffset();
          if(scope==0){
            insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
          }
          else{
            insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
          }

          struct expr *new_member;
          new_member=(struct expr*)malloc(sizeof(struct expr));
          struct SymbolTableEntry *sym1;
          sym1=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
          new_member->sym=sym1;
          new_member->type=tableitem_e;
          new_member->sym->name=$3;
          temp->index=new_member;


          if(is_empty_queue()==1){
            emit(tablegetelem,$1,new_member,temp,0,yylineno);
            enqueue_table_queue(temp);
          }
          else{
            emit(tablegetelem,dequeue_table_queue(),new_member,temp,0,yylineno);
          }

          $$=temp;
        }
      |		lvalue L_BRACKET expr R_BRACKET  {         //pinakas
            fprintf(GOUT,"member: lvalue[expr]\n");
            struct expr *temp;
            temp=(struct expr*)malloc(sizeof(struct expr));
            struct expr *index;
            index=(struct expr*)malloc(sizeof(struct expr));
            struct SymbolTableEntry *sym;
            sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

            temp->index=index;
            temp->sym=sym;
            temp->type=var_e;
            temp->sym->offset=currScopeOffset();
            temp->sym->space=currScopeSpace();
            temp->sym->name=temp_name();
            incCurrScopeOffset();
            if(scope==0){
              insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
            }
            else{
              insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
            }

            struct expr *new_member;
            new_member=(struct expr*)malloc(sizeof(struct expr));
            struct SymbolTableEntry *sym1;
            sym1=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
            new_member->sym=sym1;
            new_member->type=tableitem_e;

            if($3->type==constnum_e){
              if($3->int_real==1){
                char sfasi[20];
                sprintf(sfasi,"%d",$3->value.intValue);
                new_member->sym->name=(char *)malloc(sizeof(strlen(sfasi)+1));
                strcpy( new_member->sym->name,sfasi);
              }
            }
            else{
                new_member->sym->name=$3->sym->name;
            }

            temp->index=new_member;

            emit(tablegetelem,$1,new_member,temp,0,yylineno);

            $$=temp;
          }
      |		call DOT IDENTIFIER  {fprintf(GOUT,"member: call.IDENTIFIER\n");}
      |		call L_BRACKET expr R_BRACKET  {fprintf(GOUT,"member: call[expr]\n");}       //pinakas
      ;

call:		lvalue callsuffix {
          fprintf(GOUT,"call: lvalue callsuffix\n");
          if(($1->type!=programfunc_e) && (!look_lib_func($1->sym->name))){
            fprintf(GOUT,"Error at line %d: Invalid call\n",yylineno);
            exit(0);
          }
          call_args_counter=0;
          emit(call,NULL,NULL,$1,0,yylineno);

          struct expr *new_ret;
          new_ret=(struct expr*)malloc(sizeof(struct expr));
          struct SymbolTableEntry *sym;
          sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
          new_ret->sym=sym;
          new_ret->type=var_e;
          new_ret->sym->name=temp_name_return();

          emit(getretval,NULL,NULL,new_ret,0,yylineno);
          $$=new_ret;
        }

    |		L_PARENTHESIS funcdef R_PARENTHESIS L_PARENTHESIS elist R_PARENTHESIS	{
          fprintf(GOUT,"call: ( funcdef ) ( elist )\n");
          call_args_counter=0;
          emit(call,NULL,NULL,$2,0,yylineno);

          struct expr *new_ret;
          new_ret=(struct expr*)malloc(sizeof(struct expr));
          struct SymbolTableEntry *sym;
          sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
          new_ret->sym=sym;
          new_ret->type=var_e;
          new_ret->sym->name=temp_name_return();

          emit(getretval,NULL,NULL,new_ret,0,yylineno);
          $$=new_ret;
        }

    | 	call L_PARENTHESIS elist R_PARENTHESIS {
          fprintf(GOUT,"call: ( elist )\n");
          call_args_counter=0;
          emit(call,NULL,NULL,$1,0,yylineno);

          struct expr *new_ret;
          new_ret=(struct expr*)malloc(sizeof(struct expr));
          struct SymbolTableEntry *sym;
          sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
          new_ret->sym=sym;
          new_ret->type=var_e;
          new_ret->sym->name=temp_name_return();

          emit(getretval,NULL,NULL,new_ret,0,yylineno);
          $$=new_ret;
        }
    ;

callsuffix:	normcall  {fprintf(GOUT,"callsuffix: normcall\n");}
    	  |	methodcall {fprintf(GOUT,"callsuffix: methodcall\n");}
    	  ;

normcall:	L_PARENTHESIS elist R_PARENTHESIS	{fprintf(GOUT,"normcall: ( elist )\n");}
    	   ;

methodcall:	DOUBLE_DOT IDENTIFIER L_PARENTHESIS elist R_PARENTHESIS {fprintf(GOUT,"methodcall: ..IDENTIFIER ( elist )\n");}
    	   ;

elist:		expr {
            fprintf(GOUT,"elist: expr\n");
            call_args_counter++;
            struct expr *new_param;
            new_param=(struct expr*)malloc(sizeof(struct expr));

            struct SymbolTableEntry *sym;
            sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
            new_param->sym=sym;

            if($1->type==var_e){
              new_param->type=var_e;
              new_param->sym->name=$1->sym->name;
            }
            else{
              if($1->int_real==0){
                new_param->value.realValue=$1->value.realValue;
                new_param->type=constnum_e;
                new_param->int_real=0;
              }
              else if($1->int_real==1){
                new_param->value.intValue=$1->value.intValue;
                new_param->type=constnum_e;
                new_param->int_real=1;
              }
              else if($1->int_real==-1){
                new_param->value.stringValue=$1->value.stringValue;
                new_param->type=conststring_e;
                new_param->int_real=-1;
              }
              else if($1->int_real==-2){
                new_param->value.boolean=$1->value.boolean;
                new_param->type=constbool_e;
                new_param->int_real=-2;
              }
            }
            elist_head=new_param;
            $$=elist_head;
          }

     |		elist COMMA expr{
            fprintf(GOUT,"elist: elist, expr\n");
            call_args_counter++;
            struct expr *new_param;
            new_param=(struct expr*)malloc(sizeof(struct expr));

            if($3->type==var_e){
              struct SymbolTableEntry *sym;
              sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
              new_param->sym=sym;
              new_param->type=var_e;
              new_param->sym->name=$3->sym->name;
            }
            else{
              if($3->int_real==0){
                new_param->value.realValue=$3->value.realValue;
                new_param->type=constnum_e;
                new_param->int_real=0;
              }
              else if($3->int_real==1){
                new_param->value.intValue=$3->value.intValue;
                new_param->type=constnum_e;
                new_param->int_real=1;
              }
              else if($3->int_real==-1){
                new_param->value.stringValue=$3->value.stringValue;
                new_param->type=conststring_e;
                new_param->int_real=-1;
              }
              else if($3->int_real==-2){
                new_param->value.boolean=$3->value.boolean;
                new_param->type=constbool_e;
                new_param->int_real=-2;
              }
            }
            struct expr* elist_ptr2;
            elist_ptr2=elist_head;
            while(elist_ptr2->next!=NULL){
              elist_ptr2=elist_ptr2->next;
            }
            elist_ptr2->next=new_param;
            $$=elist_head;
          }

     |    {fprintf(GOUT,"elist: \n");}
     ;

objectdef:	L_BRACKET elist R_BRACKET  {
              fprintf(GOUT,"objectdef: [ elist ]\n");

              struct expr *temp;
              temp=(struct expr*)malloc(sizeof(struct expr));
              struct SymbolTableEntry *sym;
              sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
              temp->sym=sym;
              temp->type=var_e;
              temp->sym->name=temp_name();
              incCurrScopeOffset();

              if(scope==0){
                insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
              }
              else{
                insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
              }

              struct expr *new_object;
              new_object=(struct expr*)malloc(sizeof(struct expr));
              struct SymbolTableEntry *sym1;
              sym1=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
              new_object->sym=sym1;
              new_object->type=newtable_e;
              new_object->sym->name=temp->sym->name;

              emit(tablecreate,NULL,NULL,temp,0,yylineno);

              struct expr* elist_ptr2;
              elist_ptr2=elist_head;

              while(elist_ptr2!=NULL){
                //pseudo-expr pou pairnei apla to elist_counter gia na fainetai sto quad
                struct expr *count_item;
                count_item=(struct expr*)malloc(sizeof(struct expr));
                count_item->type=constnum_e;
                count_item->value.intValue=elist_counter;
                count_item->int_real=1;
                count_item->value.intValue=elist_counter++;

                emit(tablesetelem,count_item,elist_ptr2,temp,0,yylineno);

                elist_ptr2=elist_ptr2->next;
              }

              $$=elist_head;

            }
    	 |	  L_BRACKET indexed R_BRACKET {
              fprintf(GOUT,"objectdef: [ indexed ]\n");

              struct expr *temp;
              temp=(struct expr*)malloc(sizeof(struct expr));
              struct SymbolTableEntry *sym;
              sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
              temp->sym=sym;
              temp->type=var_e;
              temp->sym->name=temp_name();
              incCurrScopeOffset();

              if(scope==0){
                insert_SymTable(temp->sym->name,scope,yylineno,1,currScopeOffset(),currScopeSpace(),0);
              }
              else{
                insert_SymTable(temp->sym->name,scope,yylineno,2,currScopeOffset(),currScopeSpace(),0);
              }

              struct expr *new_object;
              new_object=(struct expr*)malloc(sizeof(struct expr));
              struct SymbolTableEntry *sym1;
              sym1=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
              new_object->sym=sym1;
              new_object->type=newtable_e;
              new_object->sym->name=temp->sym->name;

              emit(tablecreate,NULL,NULL,temp,0,yylineno);

              struct expr* index_ptr2;
              index_ptr2=index_head;

              while(index_ptr2!=NULL){
                emit(tablesetelem,index_ptr2,index_ptr2->index,temp,0,yylineno);
                index_ptr2=index_ptr2->next;
              }

              $$=new_object;
          }

    	 ;

indexed:	indexedelem  {
             fprintf(GOUT,"indexedelem\n");
             index_head=$1;
             $$=$1;
         }
	   | 	indexed COMMA indexedelem  {
             fprintf(GOUT,"indexed: indexed , indexedelem\n");
             struct expr* index_ptr;
             index_ptr=index_head;
             while(index_ptr->next!=NULL){
               index_ptr=index_ptr->next;
             }
             index_ptr->next=$3;
             $$=index_head;
         }
	   ;

indexedelem:	L_CURLY expr COLON expr R_CURLY	{
                fprintf(GOUT,"indexedelem: { expr : expr }\n");
                if($2->value.stringValue!=NULL){
                  if($4->type==programfunc_e){
                    change_name($4->sym->name,$2->value.stringValue,scope);
                  }
                }

                struct expr *new_indexedelem;
                new_indexedelem=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym;
                sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                new_indexedelem->sym=sym;
                new_indexedelem=$2;

                struct expr *new_index;
                new_index=(struct expr*)malloc(sizeof(struct expr));
                struct SymbolTableEntry *sym1;
                sym1=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                new_indexedelem->index=new_index;
                new_indexedelem->index=$4;

                $$=new_indexedelem;
              }
		   ;

block:		L_CURLY {
            fprintf(GOUT,"block: { program }\n");
  					scope++;

				  }

				  program R_CURLY{
            HideVar(scope);
            scope--;

          }
     ;

funcdef:	FUNCTION IDENTIFIER {
		   if(look_lib_func($2)){ //first check for libfunc colission.
			   fprintf(GOUT,"Error at line %d: Invalid function name,colission with library function: %s\n", yylineno,$2);
			   exit(0);
		   }
		   //Check for variables and functions in current scope.
		   if( lookup_symTable($2,scope,1) || lookup_symTable($2,scope,2) || lookup_symTable($2,scope,3) || lookup_symTable($2,scope,4) ){
			   fprintf(GOUT,"Error at line %d: Invalid function name, identifier: %s already exists.\n", yylineno,$2);
			   exit(0);
		   }
		   //If nothing found, then insert new function in SymbolTable
		   else{
         struct expr *new_func;
         new_func=(struct expr*)malloc(sizeof(struct expr));
         struct SymbolTableEntry *sym;
         sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
         new_func->sym=sym;

         new_func->sym->name=$2;
         new_func->type=programfunc_e;
         new_func->value.intValue=currQuad;

			   inside_function++;
         incCurrScopeOffset();
         push(currScopeOffset());
         enterScopeSpace();

         f_push(new_func->sym->name,currQuad);
         insert_funcstart_list(currQuad);
         emit(jump,NULL,NULL,NULL,0,yylineno);
         emit(funcstart,NULL,NULL,new_func,0,yylineno);
		   }
    } L_PARENTHESIS idlist R_PARENTHESIS {push(currScopeOffset()); enterScopeSpace();} block {

				fprintf(GOUT,"funcdef: function IDENTIFIER ( idlist ) block\n");

        unsigned func_locs;
        unsigned func_lel;

        struct expr *new_func;
        new_func=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        new_func->sym=sym;



        //mia pop() gia ta locals, mia gia ta idlist
        pop();
        func_lel=pop();
        func_locs=currScopeOffset()-func_lel;
        restoreCurScopeOffset(func_lel);
        exitScopeSpace();
        exitScopeSpace();

        Function_stack* temp_func=f_pop();
        new_func->sym->name=temp_func->name;
        new_func->value.intValue=temp_func->label;
        new_func->type=programfunc_e;
        new_func->sym->offset=currScopeOffset();
        new_func->sym->space=currScopeSpace();
        $$=new_func;

        //Insert new function in SymbolTable
        insert_SymTable($2,scope,yylineno,4,currScopeOffset(),currScopeSpace(),func_locs);
				inside_function--;

        emit(funcend,NULL,NULL,new_func,0,yylineno);
        backpatch_funcstart_list(currQuad);

		}
		|	FUNCTION {
				inside_function++;

        struct expr *new_func;
        new_func=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        new_func->sym=sym;

        new_func->sym->name=temp_name_func();
        new_func->type=programfunc_e;
        new_func->value.intValue=currQuad;

        incCurrScopeOffset();
        push(currScopeOffset());
        enterScopeSpace();

        f_push(new_func->sym->name,currQuad);

        insert_funcstart_list(currQuad);
        emit(jump,NULL,NULL,NULL,0,yylineno);
        emit(funcstart,NULL,NULL,new_func,0,yylineno);

		}	L_PARENTHESIS idlist R_PARENTHESIS {push(currScopeOffset()); enterScopeSpace();}block {

				fprintf(GOUT,"funcdef: function ( idlist )\n");

        unsigned func_locs;
        unsigned func_lel;
        struct expr *new_func;

        new_func=(struct expr*)malloc(sizeof(struct expr));
        struct SymbolTableEntry *sym;
        sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        new_func->sym=sym;

        //mia pop() gia ta locals, mia gia ta idlist
        pop();
        func_lel=pop();
        func_locs=currScopeOffset()-func_lel;
        restoreCurScopeOffset(func_lel);
        exitScopeSpace();
        exitScopeSpace();

        Function_stack* temp_func=f_pop();
        new_func->sym->name=temp_func->name;
        new_func->value.intValue=temp_func->label;
        new_func->type=programfunc_e;
        new_func->sym->offset=currScopeOffset();
        new_func->sym->space=currScopeSpace();
        $$=new_func;

        //Insert new function in SymbolTable
        insert_SymTable(new_func->sym->name,scope,yylineno,4,currScopeOffset(),currScopeSpace(),func_locs);
        inside_function--;
        emit(funcend,NULL,NULL,new_func,0,yylineno);
        backpatch_funcstart_list(currQuad);
	}
	   ;

 const:		INTEGER	{
                    fprintf(GOUT,"const: INTEGER\n");
                    struct expr *new_int;
                    new_int=(struct expr*)malloc(sizeof(struct expr));
                    struct SymbolTableEntry *sym;
                    sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                    new_int->sym=sym;
                    new_int->value.intValue =$1;
                    new_int->type=constnum_e;
                    new_int->int_real=1;
                    char s[20];
                    sprintf(s,"%d",$1);
                    new_int->sym->name=s;
                    $$=new_int;
                  }
      |		REAL		{
                    fprintf(GOUT,"const: REAL\n");
                    struct expr *new_real;
                    new_real=(struct expr*)malloc(sizeof(struct expr));
                    struct SymbolTableEntry *sym;
                    sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                    new_real->sym=sym;
                    new_real->value.realValue =$1;
                    new_real->type=constnum_e;
                    new_real->int_real=0;
                    char s[20];
                    sprintf(s,"%f",$1);
                    new_real->sym->name=s;
                    $$=new_real;
                  }
      |		STRINGLITERAL	{
                          fprintf(GOUT,"const: STRINGLITERAL\n");
                          struct expr *new_string;
                          char *csgo;
                          new_string=(struct expr*)malloc(sizeof(struct expr));
                          struct SymbolTableEntry *sym;
                          sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                          new_string->sym=sym;
                          csgo= malloc(strlen($1+1));
                          csgo=$1;
                          new_string->value.stringValue=csgo;
                          new_string->type=conststring_e;
                          new_string->int_real=-1;
                          new_string->sym->name=csgo;
                          $$=new_string;
                        }
      |		NIL				{
                      fprintf(GOUT,"const: NIL\n");
                      struct expr *new_null;
                      new_null=(struct expr*)malloc(sizeof(struct expr));
                      struct SymbolTableEntry *sym;
                      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_null->sym=sym;
                      new_null->value.boolean=0;
                      new_null->type=nil_e;
                      new_null->int_real=-2;
                      new_null->sym->name="nill";
                      $$=new_null;
                    }
      |		TRUE			{
                      fprintf(GOUT,"const: TRUE\n");
                      struct expr *new_boolean;
                      new_boolean=(struct expr*)malloc(sizeof(struct expr));
                      struct SymbolTableEntry *sym;
                      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_boolean->sym=sym;
                      new_boolean->value.boolean=1;
                      new_boolean->type=constbool_e;
                      new_boolean->int_real=-2;
                      new_boolean->sym->name="true";
                      $$=new_boolean;
                    }
      |		FALSE			{
                      fprintf(GOUT,"const: FALSE\n");
                      struct expr *new_boolean;
                      new_boolean=(struct expr*)malloc(sizeof(struct expr));
                      struct SymbolTableEntry *sym;
                      sym=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_boolean->sym=sym;
                      new_boolean->value.boolean=0;
                      new_boolean->type=constbool_e;
                      new_boolean->int_real=-2;
                      new_boolean->sym->name="false";
                      $$=new_boolean;
                    }
      ;

idlist:		IDENTIFIER {
				fprintf(GOUT,"idlist: IDENTIFIER\n");

				if(look_lib_func($1)){ //first check for libfunc colission.
					fprintf(GOUT,"Error at line %d: Invalid argument name,colission with library function: %s\n", yylineno,$1);
					exit(0);
				}
				int found = lookup_funcArgs(scope,$1);
				if(found ==1){
					fprintf(GOUT,"Error at line %d: Invalid formal name, identifier: %s already exists as function argument.\n", yylineno,$1);
					exit(0);
				}
				else{
          incCurrScopeOffset();
					insert_SymTable($1,scope+1,yylineno,3,currScopeOffset(),currScopeSpace(),0);
				}
			}
      |		idlist COMMA IDENTIFIER	{
				fprintf(GOUT,"idlist: idlist , IDENTIFIER\n");

				if(look_lib_func($3)){ //first check for libfunc colission.
					fprintf(GOUT,"Error at line %d: Invalid argument name,colission with library function: %s\n", yylineno,$3);
					exit(0);
				}

				int found = lookup_funcArgs(scope,$3); //paizoume me to <type> tou idlist mesw tou struct

			  if(found ==1){
					fprintf(GOUT,"Error at line %d: Invalid formal name, identifier: %s already exists as function argument.\n", yylineno,$3);
					exit(0);
				}
				else{
          incCurrScopeOffset();
					insert_SymTable($3,scope+1,yylineno,3,currScopeOffset(),currScopeSpace(),0);
				}
			}

	  |			{fprintf(GOUT,"idlist: \n");}
      ;

ifprefix:	IF L_PARENTHESIS expr R_PARENTHESIS stmt {
            fprintf(GOUT,"ifprefix: if ( expr ) stmt\n");
            if(($3->type==var_e) || ($3->type!=boolexpr_e)){
              fprintf(GOUT,"Error at line %d: Invalid IF condition\n",yylineno);
              exit(0);
            }
            emit(jump,NULL,NULL,NULL,0,yylineno);
            insert_jump_list(currQuad-1);
            $3->value.intValue=jump_head->label;
            $$=$3;
          }
	    ;

ifstmt: 	ifprefix ELSE stmt {
            fprintf(GOUT,"ifstmt: if ( expr ) stmt else stmt\n");
            insert_jump_list(currQuad);
            $$=$1;
          }
      ;

whilestmt:	WHILE L_PARENTHESIS expr  R_PARENTHESIS {inside_loop++;} stmt{
      				fprintf(GOUT,"whilestmt: while ( expr ) stmt\n");
              if(($3->type==var_e) || ($3->type!=boolexpr_e)){
                fprintf(GOUT,"Error at line %d: Invalid WHILE condition\n",yylineno);
                exit(0);
              }
              $$=$3;
      				inside_loop--;
      			}
    	 ;

forstmt:	FOR L_PARENTHESIS elist SEMICOLON expr SEMICOLON elist R_PARENTHESIS {inside_loop++;} stmt	{
				fprintf(GOUT,"forstmt: for ( elist; expr; elist ) stmt\n");
        if(($5->type==var_e) || ($5->type!=boolexpr_e)){
          fprintf(GOUT,"Error at line %d: Invalid FOR condition\n",yylineno);
          exit(0);
        }
        $$=$5;
				inside_loop--;
			}
       ;

returnstmt:	RETURN SEMICOLON{
              fprintf(GOUT,"returnstmt: return ;\n");
              if(inside_function==0){
                fprintf(GOUT,"Error at line %d: Invalid Return statement\n", yylineno);
                exit(0);
              }
              struct expr *new_ret;
              new_ret=(struct expr*)malloc(sizeof(struct expr));
              emit(Return,NULL,NULL,NULL,0,yylineno);
            }

    	  | RETURN expr SEMICOLON	{
            fprintf(GOUT,"returnstmt: return expr ;\n");
            if(inside_function==0){
              fprintf(GOUT,"Error at line %d: Invalid Return statement\n", yylineno);
              exit(0);
            }
            emit(Return,NULL,NULL,$2,0,yylineno);
          }
    	  ;
%%

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void yyerror(const char *s){
  if(s=="EOF"){
    fprintf(GOUT, "<EOF \n");
    exit(0);
  }else{
    if(strcmp(s,"syntax error, unexpected R_PARENTHESIS")==0){
      fprintf(GOUT, "Error at line %d unexpected R_PARENTHESIS\n",yylineno);
      exit(0);
    }
    else if(strcmp(s,"syntax error, unexpected SEMICOLON")==0){
      fprintf(GOUT, "Error at line %d unexpected SEMICOLON\n",yylineno);
      exit(0);
    }
    else{
      fprintf(GOUT, "%s: at line %d \n",s,yylineno);
      exit(0);
    }
  }
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
