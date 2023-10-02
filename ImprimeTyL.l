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

"if"                        { printf("\nToken: IF         Lexema: %s", yytext); }
"else"                      { printf("\nToken: ELSE       Lexema: %s", yytext); }
"while"                     { printf("\nToken: WHILE      Lexema: %s", yytext); }
"foreach"                   { printf("\nToken: FOREACH    Lexema: %s", yytext); }
"set"                       { printf("\nToken: SET        Lexema: %s", yytext); }
"list"                      { printf("\nToken: LIST       Lexema: %s", yytext); }
"pop"                       { printf("\nToken: POP        Lexema: %s", yytext); }
"push"                      { printf("\nToken: PUSH       Lexema: %s", yytext); }
"to"                        { printf("\nToken: TO         Lexema: %s", yytext); }
"in"                        { printf("\nToken: IN         Lexema: %s", yytext); }
"=="                        { printf("\nToken: ISEQUAL    Lexema: %s", yytext); }
"!="                        { printf("\nToken: NOTEQUAL   Lexema: %s", yytext); }
"&&"                        { printf("\nToken: AND        Lexema: %s", yytext); }
"||"                        { printf("\nToken: OR         Lexema: %s", yytext); }
"!"                         { printf("\nToken: NOT        Lexema: %s", yytext); }

"|"                         { printf("\nToken: UNION      Lexema: %s", yytext); }
"&"                         { printf("\nToken: INTERS     Lexema: %s", yytext); }
"&!"                        { printf("\nToken: DIFF       Lexema: %s", yytext); }

"[" |
"]" |
"{" |
"}" |
"(" |
")" |
"," |
";" |
"="                         { printf("\nToken: %c          Lexema: %s", yytext[0], yytext); }

"'"[a-zA-Z0-9]+             { printf("\nToken: ELEM       Lexema: %s", yytext); }
[a-zA-Z][a-zA-Z0-9]*        { printf("\nToken: ID         Lexema: %s", yytext); }

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