
/* calculator with AST */

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "fb3-2.h"
#include "TDataType.h"

int yylex(void);
%}

%union {
 struct ast *a;
 double d;
 struct symbol *s; /* which symbol */
 struct symlist *sl;
 int fn; /* which function */
 char* c;
 Tree data;
}

/* declare tokens */
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL
%token IF THEN ELSE WHILE DO LET FOREACH POP PUSH TO IN
%nonassoc <fn> LOGICOP SETOP
%nonassoc <fn> CMP //AGREGAMOS SETOP, LOGICOP y NOT
%nonassoc <fn> NOT
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
%type <a> exp stmt list explist
%type <sl> symlist
%start calclist

/* AGREGADOS */
%token <c> ELEM

%%

stmt
  : IF exp THEN list { $$ = newflow('I', $2, $4, NULL); }
  | IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
  | WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
  | exp
;

list
  : /* nothing */ { $$ = NULL; }
  | stmt ';' list {
    if ($3 == NULL)
    $$ = $1;
    else
    $$ = newast('L', $1, $3);
  }
;

exp
  : exp CMP exp                 { $$ = newcmp($2, $1, $3); }
  | exp '+' exp                 { $$ = newast('+', $1,$3); }
  | exp '-' exp                 { $$ = newast('-', $1,$3); }
  | exp '*' exp                 { $$ = newast('*', $1,$3); }
  | exp '/' exp                 { $$ = newast('/', $1,$3); }
  | '|' exp                     { $$ = newast('|', $2, NULL); }
  | '(' exp ')'                 { $$ = $2; }
  | '-' exp %prec UMINUS        { $$ = newast('M', $2, NULL); }
  | NUMBER                      { $$ = newnum($1); }
  | NAME                        { $$ = newref($1); }
  | NAME '=' exp                { $$ = newasgn($1, $3); }
  | FUNC '(' explist ')'        { $$ = newfunc($1, $3); }
  | NAME '(' explist ')'        { $$ = newcall($1, $3); }
  | '{' '}'                     { $$ = newast(SETAST, NULL, NULL); }
  | '{' explist '}'             { $$ = newast(SETAST, $2, NULL); }
  | '[' ']'                     { $$ = newast(LISTAST, NULL, NULL); }
  | '[' explist ']'             { $$ = newast(LISTAST, $2, NULL); }
  | exp SETOP exp               { $$ = newast($2, $1, $3); } // TODO: corregir, asemejar a LOGICOP
  | ELEM                        { $$ = newelem($1); }
  | exp LOGICOP exp             { $$ = newlogicop($2, $1, $3); }
  | NOT exp                     { $$ = newlogicop($1, $2, NULL); }
  /* TODO: agregar pop y positionElem */
;

explist
  : exp
  | exp ',' explist { $$ = newast('L', $1, $3); }
;

symlist
  : NAME { $$ = newsymlist($1, NULL); }
  | NAME ',' symlist { $$ = newsymlist($1, $3); }
;

calclist
  : /* nothing */
  | calclist stmt EOL
  {
    printf("= %4.4g\n> ", ValueDT(eval($2)));
    treefree($2);
  }
  | calclist LET NAME '(' symlist ')' '=' list EOL
  {
    dodef($3, $5, $8);
    printf("Defined %s\n> ", $3->name);
  }
  | calclist error EOL { yyerrok; printf("> "); }
;

%%

void yyerror(char *s, ...)
{
 va_list ap;
 va_start(ap, s);
 fprintf(stderr, "%d: error: ", yylineno);
 vfprintf(stderr, s, ap);
 fprintf(stderr, "\n");
}

int main(void)
{
 printf("> ");
 return yyparse();
}
