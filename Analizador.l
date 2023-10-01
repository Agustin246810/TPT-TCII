%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(char *s);

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
  IN = 268,
  ID = 269,
  EOL = 270,
  ISEQUAL = 271,
  NOTEQUAL = 272,
  AND = 273,
  OR = 274,
  UNION = 275,
  INTERS = 276,
  DIFF = 277
};

%}

%option noyywrap
/* Permite obtener el numero de linea */
%option yylineno

%%

"if"                        { return IF; }
"else"                      { return ELSE; }
"while"                     { return WHILE; }
"foreach"                   { return FOREACH; }
"set"                       { return SET; }
"list"                      { return LIST; }
"pop"                       { return POP; }
"push"                      { return PUSH; }
"to"                        { return TO; }
"in"                        { return IN; }
"=="                        { return ISEQUAL; }
"=!"                        { return NOTEQUAL; }
"&&"                        { return AND; }
"||"                        { return OR; }

"|"                         { return UNION; }
"&"                         { return INTERS; }
"&!"                        { return DIFF; }

("'")([a-zA-Z0-9]+)         { return ELEM; }
([a-zA-z])([a-zA-Z0-9]*)    { return ID; }

"{" |
"}" |
"[" |
"]" |
"(" |
")" |
"," |
";" |
"="                         { return yytext[0]; }

"\n"                        { return EOL; }
"//".*                      { /* Ignora comentarios */ }
[ \t]                       { /* Ignora espacios en blanco */ }
.                           yyerror("\nERROR Caracter Desconocido");

%%
