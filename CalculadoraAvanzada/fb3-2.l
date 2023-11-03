/* recognize tokens for the calculator */
%option noyywrap nodefault yylineno
%{
# include "fb3-2.h"
# include "fb3-2.tab.h"
%}

/* float exponent */
EXP ([Ee][-+]?[0-9]+)

%%
 /* single character ops */
"{" |
"}" |
"[" |
"]" |
"+" |
"-" |
"*" |
"/" |
"=" |
 /* "%" | // cambio de "|" a "%" para el valor absoluto */
"," |
";" |
":" | // estaContenidoEn TODO: cambiar a ":"
"(" |
")" { return yytext[0]; }

 /* comparison ops, all are a CMP token */
">"                     { yylval.fn = GREATEROP;          return CMP; }
"<"                     { yylval.fn = LESSEROP;           return CMP; }
"!="                    { yylval.fn = NOTEQUALOP;         return CMP; }
"=="                    { yylval.fn = ISEQUALOP;          return CMP; }
">="                    { yylval.fn = GREATEROREQUALOP;   return CMP; }
"<="                    { yylval.fn = LESSEROREQUALOP;    return CMP; }

"|"                     { yylval.fn = UNIONOP;            return SETOP; }
"&"                     { yylval.fn = INTERSOP;           return SETOP; }
"&!"                    { yylval.fn = DIFFOP;             return SETOP; }

"&&"                    { yylval.fn = ANDOP;              return LOGICOP; }
"||"                    { yylval.fn = OROP;               return LOGICOP; }
"!"                     { yylval.fn = NOTOP;              return NOT; }

 /* literal element */
"'"[a-zA-Z0-9]+         { yylval.c = yytext; return ELEM;} // La cadena se guarda junto con el '

 /* keywords */
"if"                    { return IF; }
 /* "then"                  { return THEN; } */
"else"                  { return ELSE; }
"while"                 { return WHILE; }
 /* "do"                    { return DO; } */
"let"                   { return LET; }
"foreach"               { return FOREACH; }
"pop"                   { return POP; }
"push"                  { return PUSH; }
"to"                    { return TO; }
"in"                    { return IN; }

 /* built-in functions */
"sqrt"                  { yylval.fn = B_sqrt;   return FUNC; }
"exp"                   { yylval.fn = B_exp;    return FUNC; }
"log"                   { yylval.fn = B_log;    return FUNC; }
"print"                 { yylval.fn = B_print;  return FUNC; }
"abs"                   { yylval.fn = B_abs;    return FUNC; }

 /* names */
[a-zA-Z][a-zA-Z0-9]*    { yylval.s = lookup(yytext); return NAME; }
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?        { yylval.d = atof(yytext); return NUMBER; }
"//".*
[ \t] /* ignore whitespace */
\\\n                    { printf("c> "); } /* ignore line continuation */
\n                      { return EOL; }
.                       { yyerror("Mystery character %c\n", *yytext); }
%%