
/*Definition Section*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


int comment_nesting = 0;
int comment_flag = 0;
int string_flag = 0;

void yyerror(char *);
void initString();
void reallocString();
void saveChar(char c);

char* string_array = NULL;
char* stringValue;
int string_max_size;
int string_index = -1;

%}
%x SC_COMMENT
%x STRING
%option yylineno

string_start	\"
single_comment		"//"+.*
whitespace		[' ' \t\n\r\f]

%%
{whitespace}	/*deletes spaces */

"if"		return IF;
"else"		return ELSE;
"while"		return WHILE;
"for"		return FOR;
"function"	return FUNCTION;
"return"	return RETURN;
"break"		return BREAK;
"continue"	return CONTINUE;
"and"		return AND;
"not"		return NOT;
"or"		return OR;
"local"		return LOCAL;
"true"		return TRUE;
"false"		return FALSE;
"nil"		return NIL;
"="			return ASSIGN;
"+"			return PLUS;
"-"			return MINUS;
"*"			return MUL;
"/"			return DIV;
"%"			return MODULO;
"="{2}		return EQUALS;
"!="		return NOT_EQUALS;
"+"{2}		return INCR;
"-"{2}		return DECR;
">"			return GREATER;
"<"			return LESS;
">="		return GR_EQUAL;
"<="		return LESS_EQUAL;
"{"			return L_CURLY;
"}"			return R_CURLY;
"["			return L_BRACKET;
"]"			return R_BRACKET;
"("			return L_PARENTHESIS;
")"			return R_PARENTHESIS;
";"			return SEMICOLON;
","			return COMMA;
":"			return COLON;
"::"		return DOUBLE_COLON;
"."			return DOT;
".."		return DOUBLE_DOT;

[0-9]*\.[0-9]*				{yylval.realValue = atof(yytext);		return REAL;}
[1-9][0-9]*|0				{yylval.intValue = atoi(yytext);		return INTEGER;}
[a-z|A-Z][a-z|A-Z|_|0-9]*	{yylval.stringValue = strdup(yytext);	return IDENTIFIER;}

{string_start}		{initString(); BEGIN(STRING);}
<STRING>
{
\" {
	BEGIN(INITIAL);
	saveChar('\0');
	yylval.stringValue = strdup(string_array);
	free(string_array);
	return STRINGLITERAL;
     }

     \n  saveChar('\n');
     \\n saveChar('\n');
     \t  saveChar('\t');
     \\t saveChar('\t');
     \\\" saveChar('\"');
     \\\\ saveChar('\\');
     \\. {fprintf(stderr,"Error in line :%d: invalid escape character\n",yylineno); }
     [^\"\n\t\\]+ {char *c = yytext; while(*c!='\0') {saveChar(*c);c++;} }
     <<EOF>> {fprintf(stderr,"Error in line :%d: Reached EOF in string\n",yylineno); exit(EXIT_FAILURE);}
}

"/*" {
  comment_flag = 1;
  BEGIN(SC_COMMENT);
}

<SC_COMMENT>[^*^/]*            ;
<SC_COMMENT>"/"+[^*]*           ;
<SC_COMMENT>"*"+[^*/]*           ;
<SC_COMMENT>"/"+"*"	comment_nesting++;
<SC_COMMENT>"*"+"/" {
			
			if(comment_nesting==0) {	
				BEGIN(INITIAL);
				return 0;
			}
			else{
			    comment_nesting--;
				return 0;
				BEGIN(SC_COMMENT);
			}
		}
		
		
<SC_COMMENT><<EOF>> {
			{fprintf(stderr,"Error in line :%d: comment left open\n",yylineno);exit(EXIT_FAILURE);}
                        BEGIN(INITIAL);
		 }


{single_comment}	{}
.			{}
<<EOF>>			{return 0;}


%%

int yywrap(void){
	return 1;
}

void initString(){
	string_index = -1;
	string_max_size = 10;
	string_array = (char*) malloc(sizeof(char) * string_max_size);
}

void reallocString(){
	string_max_size += string_max_size/2;
	string_array = (char*) realloc(string_array,sizeof(char) * string_max_size);
}

void saveChar(char c){
	if(string_index < string_max_size-1){
		string_array[++string_index] = c;
	}
	else{
		reallocString();
		saveChar(c);
	}
}