%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quads.h"
#include "Stack.h"

extern int yylineno;
extern char *yytext;
extern FILE *yyin;
FILE *GOUT;
int func_name = 0;
unsigned int scope = 0;
int inside_function=0;
int inside_loop=0;
int lvalue_flag=0;
int const_flag=0;
int func_flag=0;
int call_args_counter=0;
char *f_name="_F";



char* concat(const char *s1, const char *s2);

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
  struct SymbolTableEntry *exprNode;
  struct FuncArg *argument_t;
};

%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL ASSIGN PLUS MINUS MUL DIV MODULO EQUALS NOT_EQUALS INCR DECR GREATER LESS GR_EQUAL LESS_EQUAL L_CURLY R_CURLY L_BRACKET R_BRACKET L_PARENTHESIS R_PARENTHESIS SEMICOLON COMMA COLON DOUBLE_COLON DOT DOUBLE_DOT
%token<intValue> INTEGER
%token<realValue> REAL
%token<stringValue> IDENTIFIER STRINGLITERAL


%type<exprNode> lvalue funcdef const assignexpr expr term primary stmt
%type<argument_t> idlist
%type<realValue>  arithmeticop
%type<stringValue> ifstmt whilestmt forstmt returnstmt block  ifprefix
%type<intValue> booleanop relativeop
%type<stringValue>  call objectdef  member
%type<stringValue> elist normcall methodcall callsuffix indexed indexedelem

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


program:	program stmt	 {fprintf(GOUT,"program: program stmt\n");}
	     |                 {fprintf(GOUT,"program: \n");}
	     ;

stmt:	expr SEMICOLON		   {fprintf(GOUT,"stmt: expr ;  \n");}
    | ifprefix        	   {fprintf(GOUT,"stmt: ifstmt\n");}
    | ifstmt          	   {fprintf(GOUT,"stmt: ifstmt\n");}
    | whilestmt       	   {fprintf(GOUT,"stmt: whilestmt\n");}
    | forstmt         	   {fprintf(GOUT,"stmt: forstmt\n");}
    | returnstmt      	   {fprintf(GOUT,"stmt: returnstmt\n");}
    | BREAK SEMICOLON 	   {
		fprintf(GOUT,"statement: break ;\n");
		if(inside_loop==0){
			fprintf(GOUT,"Error at line %d: Break statement not inside a loop.\n", yylineno);
            exit(0);
		}
	}
    | CONTINUE SEMICOLON	 {
		fprintf(GOUT,"statement: continue ;\n");
		if(inside_loop==0){
			fprintf(GOUT,"Error at line %d: Continue statement not inside a loop.\n", yylineno);
            exit(0);
		}
	}
    | block   {fprintf(GOUT,"stmt: block\n");}
    | funcdef         	   {fprintf(GOUT,"stmt: funcdef\n");}
    | SEMICOLON       	   {fprintf(GOUT,"stmt: SEMICOLON\n");}
	  ;

expr:	assignexpr     		   {fprintf(GOUT,"expr: assignexpr\n");}
    | arithmeticop	   	   {fprintf(GOUT,"expr: arithmeticop\n");}
    | booleanop		         {fprintf(GOUT,"expr: booleanop\n");}
    | relativeop		   	   {fprintf(GOUT,"expr: relativeop\n");}
    | term {
        fprintf(GOUT,"expr: term\n");
        if(lvalue_flag==1){
          $$=$1;
          lvalue_flag=0;
        }
        else if(const_flag==1){
          $$=$1;
          const_flag=0;
        }
        else if(func_flag==1){
          $$=$1;
          func_flag=0;
        }
      }
    ;

arithmeticop: expr PLUS expr			{fprintf(GOUT,"op: expr + expr\n");}
  			   |  	expr MINUS expr		{fprintf(GOUT,"op: expr - expr\n");}
  			   |	expr MUL expr		{fprintf(GOUT,"op: expr * expr\n");}
			     | 	expr DIV expr		{fprintf(GOUT,"op: expr / expr\n");}
			     | 	expr MODULO expr	{fprintf(GOUT,"op: expr Modulo expr\n");}
			     ;

relativeop:		expr GREATER expr   {fprintf(GOUT,"op: expr > expr\n");}
		  |		expr GR_EQUAL expr	{fprintf(GOUT,"op: expr >= expr\n");}
		  |		expr LESS expr		{fprintf(GOUT,"op: expr < expr\n");}
		  |		expr LESS_EQUAL expr{fprintf(GOUT,"op: expr <= expr\n");}
		  |		expr EQUALS expr	{fprintf(GOUT,"op: expr == expr\n");}
		  |		expr NOT_EQUALS expr{fprintf(GOUT,"op: expr != expr\n");}
		  ;

booleanop:		expr AND expr		{fprintf(GOUT,"op: expr && expr\n");}
		 |		expr OR expr		{fprintf(GOUT,"op: expr || expr\n");}
		 ;

term:		L_PARENTHESIS expr R_PARENTHESIS 	{fprintf(GOUT,"term: ( expr )\n");}
	|		MINUS expr %prec UMINUS				{fprintf(GOUT,"term: -expr\n");}
	|		NOT expr							{fprintf(GOUT,"term: ! expr\n");}
	|		INCR lvalue{
        fprintf(GOUT,"term: ++lvalue\n");
        if($2->type==4 || $2->type==5){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$2->name);
          exit(0);
        }
      }
	|		lvalue INCR	{
        fprintf(GOUT,"term: lvalue++\n");
        if($1->type==4 || $1->type==5){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$1->name);
          exit(0);
        }
      }
	|		DECR lvalue{
        fprintf(GOUT,"term: --lvalue\n");
        if($2->type==4 || $2->type==5){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$2->name);
          exit(0);
        }
      }
	|		lvalue DECR{
        fprintf(GOUT,"term: lvalue--\n");
        if($1->type==4 || $1->type==5){
          fprintf(GOUT,"Error at line %d: Invalid action to function %s \n",yylineno,$1->name);
          exit(0);
        }
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
              if($1->type==4 || $1->type==5){
                fprintf(GOUT,"Error at line %d: Cannot assign to Funtion %s \n",yylineno,$1->name);
                exit(0);
              }
              if($3->type==4 || $3->type==5){
                $1->type=$3->type;
                change_type($1->name);
                $$=$1;
              }
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
	   |	  objectdef {fprintf(GOUT,"primary: objectdef\n");}

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

      struct SymbolTableEntry *temp_maloc;
      temp_maloc=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

			//arxika check an einai libfunc
			if(look_lib_func($1)){
				type=5;
				temp_maloc->name=$1;
				temp_maloc->type=type;
				$$=temp_maloc;
				break;
			}
			//psaxnoume ta scope apo mesa pros ta eksw
			while(tmp_scope >= 0){
				//periptwseis pou vriskoume kati
				if((found_scope=lookup_symTable2($1,tmp_scope,1))!=-2){
					type=1;
					temp_maloc->name=$1;
					temp_maloc->type=type;
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
					temp_maloc->name=$1;
					temp_maloc->type=type;
					$$=temp_maloc;
    				break;
    			}
    			else if((found_scope=lookup_symTable2($1,tmp_scope,3))!=-2){
    				type=3;
    				if(scope==found_scope){
						temp_maloc->name=$1;
						temp_maloc->type=type;
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
					  temp_maloc->name=$1;
					  temp_maloc->type=type;
					  $$=temp_maloc;
    				break;
    			}
				tmp_scope--;
			}
			//an dn vrethei tipota kanoume insert sto trexon scope
			if(type==0){
    			if(scope == 0){
    				insert_SymTable($1,scope,yylineno,1);
            incCurrScopeOffset();
					  temp_maloc->name=$1;
					  temp_maloc->type=1;
					  $$=temp_maloc;
    			}
    			else{
    				insert_SymTable($1,scope,yylineno,2);
            incCurrScopeOffset();
					  temp_maloc->name=$1;
					  temp_maloc->type=2;
					  $$=temp_maloc;
    			}
    		}
	  }
	  |		LOCAL IDENTIFIER		{
			fprintf(GOUT,"lvalue: local IDENTIFIER\n");

			struct SymbolTableEntry *temp_maloc;
			temp_maloc=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

			if(look_lib_func($2)){
				fprintf(GOUT,"Error at line %d: Local identifier trying to shadow library function: %s\n", yylineno,$2);
				exit(0);
			}

			//psaxnoume sto trexon scope
			if( lookup_symTable($2,scope,2)){
				temp_maloc->name=$2;
				temp_maloc->type=2;
				$$=temp_maloc;
				break;
			}
			else if(lookup_symTable($2,scope,4)){
				temp_maloc->name=$2;
				temp_maloc->type=4;
				$$=temp_maloc;
				break;
			}
			//kanoume eisagwgh ston ST agnowntas to
			//keyword "local" an eimaste se scope 0
			else{
				if(scope == 0){
					temp_maloc->name=$2;
					temp_maloc->type=1;
					$$=temp_maloc;
					insert_SymTable($2,0,yylineno,1);
          incCurrScopeOffset();
				}
				else{
					temp_maloc->name=$2;
					temp_maloc->type=2;
					$$=temp_maloc;
					insert_SymTable($2,scope,yylineno,2);
          incCurrScopeOffset();
				}
			}
	  }
	  |		DOUBLE_COLON IDENTIFIER	{
			fprintf(GOUT,"lvalue: ::IDENTIFIER\n");

			struct SymbolTableEntry *temp_maloc;
			temp_maloc=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));

			if(lookup_symTable($2,0,1)){
				temp_maloc->name=$2;
				temp_maloc->type=1;
				$$=temp_maloc;
				break;
			}
			else if(lookup_symTable($2,0,4)){
				temp_maloc->name=$2;
				temp_maloc->type=4;
				$$=temp_maloc;
				break;
			}
			else{
				fprintf(GOUT,"Error at line %d: Invalid reference.No global identifier: %s\n", yylineno,$2);
				exit(0);
			}
	  }
	  |		member			{fprintf(GOUT,"lvalue: member\n");}
      ;

member:		lvalue DOT IDENTIFIER			{fprintf(GOUT,"member: lvalue.IDENTIFIER\n");}
      |		lvalue L_BRACKET expr R_BRACKET	{fprintf(GOUT,"member: lvalue[expr]\n");}   //pinakas
      |		call DOT IDENTIFIER				{fprintf(GOUT,"member: call.IDENTIFIER\n");}
      |		call L_BRACKET expr R_BRACKET	{fprintf(GOUT,"member: call[expr]\n");}       //pinakas
      ;

call:		lvalue callsuffix {
          fprintf(GOUT,"call: lvalue callsuffix\n");
          if(($1->type!=4) && (!look_lib_func($1->name))){
            fprintf(GOUT,"Error at line %d: Invalid call\n",yylineno);
            exit(0);
          }
        // delete_call_args(scope,call_args_counter);

          call_args_counter=0;
        }
    |		L_PARENTHESIS funcdef R_PARENTHESIS L_PARENTHESIS elist R_PARENTHESIS	{
          fprintf(GOUT,"call: ( funcdef ) ( elist )\n");
        //  delete_call_args(scope,call_args_counter);
          call_args_counter=0;
        }
    | 	call L_PARENTHESIS elist R_PARENTHESIS {
          fprintf(GOUT,"call: ( elist )\n");
        //  delete_call_args(scope,call_args_counter);
          call_args_counter=0;
        }
    ;

callsuffix:	normcall {fprintf(GOUT,"callsuffix: normcall\n");}
    	  |	methodcall	{fprintf(GOUT,"callsuffix: methodcall\n");}
    	  ;

normcall:	L_PARENTHESIS elist R_PARENTHESIS	{fprintf(GOUT,"normcall: ( elist )\n");}
    	   ;

methodcall:	DOUBLE_DOT IDENTIFIER L_PARENTHESIS elist R_PARENTHESIS {fprintf(GOUT,"methodcall: ..IDENTIFIER ( elist )\n");}
    	   ;

elist:		expr {
            fprintf(GOUT,"elist: expr\n");
            call_args_counter++;
          }
     |		elist COMMA expr{
            fprintf(GOUT,"elist: elist, expr\n");
            call_args_counter++;
          }
     |    {fprintf(GOUT,"elist: \n");}
     ;

objectdef:	L_BRACKET elist R_BRACKET		  {fprintf(GOUT,"objectdef: [ elist ]\n");}
    	 |	  L_BRACKET indexed R_BRACKET		{fprintf(GOUT,"objectdef: [ indexed ]\n");}
    	 ;

indexed:	indexedelem					       {fprintf(GOUT,"indexedelem\n");}
	   | 	indexed COMMA indexedelem 	 {fprintf(GOUT,"indexed: indexed , indexedelem\n");}
	   ;

indexedelem:	L_CURLY expr COLON expr R_CURLY	{
                fprintf(GOUT,"indexedelem: { expr : expr }\n");
                if($2->value.stringValue!=NULL){
                  if($4->type==4){
                    change_name($4->name,$2->value.stringValue,scope);
                  }
                }
              }
		   ;

block:		L_CURLY {
            fprintf(GOUT,"block: { program }\n");
  					scope++;
				  }
				  program R_CURLY{
            HideVar(scope--);
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
				   inside_function++;
			     insert_SymTable($2,scope,yylineno,4);
           incCurrScopeOffset();
           push(currScopeOffset());
           enterScopeSpace();
		   }
    } L_PARENTHESIS idlist R_PARENTHESIS {push(currScopeOffset()); enterScopeSpace();} block {

				fprintf(GOUT,"funcdef: function IDENTIFIER ( idlist ) block\n");


        struct SymbolTableEntry *new_func;
        new_func=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        new_func->name=$2;
        new_func->type=4;
        $$=new_func;
				inside_function--;
        printf("!!!!!!!!!!!!!!!!!!  %d\n",currScopeOffset());
        exitScopeSpace();
        printf("!!!!!!!!!!!!!!!!!!  %d\n",currScopeOffset());
        exitScopeSpace();
        //mia pop() gia ta locals, mia gia ta idlist
        pop();
        restoreCurScopeOffset(pop());
        printf("!!!!!!!!!!!!!!!!!!  %d\n",currScopeOffset());
		}
		|	FUNCTION {
				inside_function++;
				int i=0;
				char *a="a";
				while(i<=func_name){
					f_name=concat(f_name, a);
					i++;
				}
				func_name++;
				//Insert new function in SymbolTable
				insert_SymTable(f_name,scope,yylineno,4);
        incCurrScopeOffset();
        push(currScopeOffset());
        enterScopeSpace();

		}	L_PARENTHESIS idlist R_PARENTHESIS {push(currScopeOffset()); enterScopeSpace();}block {

				fprintf(GOUT,"funcdef: function ( idlist )\n");
        struct SymbolTableEntry *new_func;
        new_func=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
        new_func->name= f_name;
        new_func->type=4;
        $$=new_func;
        inside_function--;
        exitScopeSpace();
        exitScopeSpace();
        //mia pop() gia ta locals, mia gia ta idlist
        pop();
        restoreCurScopeOffset(pop());
	}
	   ;

 const:		INTEGER	{
                    fprintf(GOUT,"const: INTEGER\n");
                    struct SymbolTableEntry *new_int;
                    new_int=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                    new_int->value.intValue =$1;
                    $$=new_int;
                  }
      |		REAL		{
                    fprintf(GOUT,"const: REAL\n");
                    struct SymbolTableEntry *new_real;
                    new_real=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                    new_real->value.realValue =$1;
                    $$=new_real;
                  }
      |		STRINGLITERAL	{
                          fprintf(GOUT,"const: STRINGLITERAL\n");
                          struct SymbolTableEntry *new_string;
                          char *csgo;
                          new_string=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                          csgo= malloc(strlen($1+1));
                          csgo=$1;
                          new_string->value.stringValue=csgo;
                          $$=new_string;
                        }
      |		NIL				{
                      fprintf(GOUT,"const: NIL\n");
                      struct SymbolTableEntry *new_null;
                      new_null=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_null->value.boolean=1;
                      $$=new_null;
                    }
      |		TRUE			{
                      fprintf(GOUT,"const: TRUE\n");
                      struct SymbolTableEntry *new_boolean;
                      new_boolean=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_boolean->value.boolean=1;
                      $$=new_boolean;
                    }
      |		FALSE			{
                      fprintf(GOUT,"const: FALSE\n");
                      struct SymbolTableEntry *new_boolean;
                      new_boolean=(struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
                      new_boolean->value.boolean=0;
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
					insert_funcArg(scope+1,yylineno,$1);
					insert_SymTable($1,scope+1,yylineno,3);
          incCurrScopeOffset();
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
					insert_funcArg(scope+1,yylineno,$3);
					insert_SymTable($3,scope+1,yylineno,3);
          incCurrScopeOffset();
				}
			}
	  |			{fprintf(GOUT,"idlist: \n");}
      ;

ifprefix:	IF L_PARENTHESIS expr R_PARENTHESIS stmt {fprintf(GOUT,"ifstmt: if ( expr ) stmt\n");}
	    ;

ifstmt: 	ifprefix ELSE stmt	{fprintf(GOUT,"ifstmt: if ( expr ) stmt else stmt\n");}
      ;

whilestmt:	WHILE L_PARENTHESIS expr  R_PARENTHESIS {inside_loop++;} stmt{
      				fprintf(GOUT,"whilestmt: while ( expr ) stmt\n");
      				inside_loop--;
      			}
    	 ;

forstmt:	FOR L_PARENTHESIS elist SEMICOLON expr SEMICOLON elist R_PARENTHESIS {inside_loop++;} stmt	{
				fprintf(GOUT,"forstmt: for ( elist; expr; elist ) stmt\n");
				inside_loop--;
			}
       ;

returnstmt:	RETURN SEMICOLON{
              fprintf(GOUT,"returnstmt: return ;\n");
              if(inside_function==0){
                fprintf(GOUT,"Error at line %d: Invalid Return statement\n", yylineno);
                exit(0);
              }
            }
    	  | RETURN expr SEMICOLON	{
            fprintf(GOUT,"returnstmt: return expr ;\n");
            if(inside_function==0){
              fprintf(GOUT,"Error at line %d: Invalid Return statement\n", yylineno);
              exit(0);
            }
          }
    	  ;
%%

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

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc, char **argv){
  //init_symTable();

  programVarOffset=0;
  functionLocalOffset=0;
  formalArgOffset=0;
  scopeSpaceCounter=1;


  FILE *ifp, *ofp;
  ifp = fopen(argv[1], "r");

  if (ifp) {
	  yyin = ifp;
  }else{
	  fprintf(stderr, "Can't open input file!\n");
	  return(1);
  }

  if (argc > 2){
	  ofp = fopen(argv[2], "wb+");
	  if (ofp){
		GOUT = ofp;
	  }
	  else{
		fprintf(stderr, "Can't open output file %s!\n", argv[2]);
		return(1);
	  }
  }else{
	  GOUT = stdout;
  }

  yyparse();

  fclose(yyin);

  if(GOUT!=stdout){
	fclose(GOUT);
  }
  print_symTable();
  return 0;
}
