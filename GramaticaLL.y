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
  : complexop exprec
;

exprec
  :
  | CMP complexop exprec
;

logicop
  : NOT exp logicoprec
  | exp logicoprec
;

logicoprec
  :
  | LOGICOP exp logicoprec
;

complexop
  : literals position complexoprec
  | POP literals complexoprec
;

complexoprec
  :
  | SETOP literals complexoprec
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

explist2
  :
  | explist
;

explist
  : exp explistrec
;

explistrec
  :
  | ',' exp explistrec
;

%%
