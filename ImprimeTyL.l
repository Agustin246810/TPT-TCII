%{
#include <stdio.h>

void yyerror();

/* TOKENS */
enum yytokentype {
  IF = 258,
  ELSE = 259,
  WHILE = 260,
  FOREACH = 261,
  ELEM = 262,
  SET = 263,
  LIST = 264,
  POP = 265,
  PUSH = 266,
  TO = 267,
  ID = 268,
  IN = 269,
  UNION = 270,
  INTERS = 271,
  DIFF = 272,
  NOT = 273,
  AND = 274,
  OR = 275,
  ISEQUAL = 276,
  NOTEQUAL = 277
};

%}

%option noyywrap

%%

"if"                        { printf("\nToken: IF"); }
"else"                      { printf("\nToken: ELSE"); }
"while"                     { printf("\nToken: WHILE"); }
"foreach"                   { printf("\nToken: FOREACH"); }
"set"                       { printf("\nToken: SET"); }
"list"                      { printf("\nToken: LIST"); }
"pop"                       { printf("\nToken: POP"); }
"push"                      { printf("\nToken: PUSH"); }
"to"                        { printf("\nToken: TO"); }
"in"                        { printf("\nToken: IN"); }
"=="                        { printf("\nToken: ISEQUAL"); }
"!="                        { printf("\nToken: NOTEQUAL"); }
"&&"                        { printf("\nToken: AND"); }
"||"                        { printf("\nToken: OR"); }
"!"                         { printf("\nToken: NOT"); }

"|"                         { printf("\nToken: UNION"); }
"&"                         { printf("\nToken: INTERS"); }
"&!"                        { printf("\nToken: DIFF"); }

"[" |
"]" |
"{" |
"}" |
"(" |
")" |
"," |
";" |
"="                         { printf("\nToken: %c", yytext[0]); }

"'"[a-zA-Z0-9]+             { printf("\nToken: ELEM"); }
[a-zA-Z][a-zA-Z0-9]*        { printf("\nToken: ID"); }

"\n"                        { /* Ignora saltos de linea */ }
"//".*                      { /* Ignora comentarios */ }
[ \t]                       { /* Ignora espacios en blanco */ }
.                           yyerror(yytext);

%%

int main(void){
  yylex();

  return 0;
}

void yyerror(){
  printf("\n\nERROR LEXICO: %s, saliendo...", yytext);
  exit(1);
}