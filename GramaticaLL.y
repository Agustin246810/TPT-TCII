/* calculator with AST */

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int yylex(void);
%}

%union {
  ast a;
  double d;
  struct symbol *s; /* which symbol */
  struct symlist *sl;
  int fn; /* which function */
  char* c;
  tData data;
}

/* declare tokens */
%token EOL
%token IF ELSE WHILE LET FOREACH PUSH TO IN
%token <fn> LOGICOP
%token <fn> SETOP
%token <fn> CMP
%token POP
%token UMINUS
%token <fn> NOT
%token <c> ELEM
%token <s> NAME
%token <d> NUMBER
%token <fn> FUNC
%type <a> exp stmt list explist
%type <sl> symlist symlistAM


%%

exp
  : exp CMP complexop
  | complexop
;

logicop
  : logicop LOGICOP exp
  | NOT exp
  | exp
;

complexop
  : literals position
  | complexop SETOP literals
  | POP literals
;

position
  :
  | '[' literals ']'
;


literals
  : literalset
  | literalist
  | NAME
  | ELEM
  | '(' exp ')'
;

literalset
  : '{' explist2 '}'
;

literalist
  : '[' explist2 ']'
;

/* se usa para las listas de expresiones en listas y conj literales */
explist2
  :
  | explist
;

explist
  : exp
  | exp ',' explist
;

%%
