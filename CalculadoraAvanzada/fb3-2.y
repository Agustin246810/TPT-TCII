
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
  tData data;
}

/* declare tokens */
%token EOL
%token IF THEN ELSE WHILE DO LET FOREACH POP PUSH TO IN
%right '='
%left <fn> LOGICOP SETOP
%left <fn> CMP
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
%nonassoc <fn> NOT
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token <c> ELEM
%type <a> exp stmt list explist
%type <sl> symlist

%start calclist

%%

stmt
  : IF exp THEN list              { $$ = newflow('I', $2, $4, NULL); }
  | IF exp THEN list ELSE list    { $$ = newflow('I', $2, $4, $6); }
  | WHILE exp DO list             { $$ = newflow('W', $2, $4, NULL); }
  | exp
;

list
  : /* nothing */                 { $$ = NULL; }
  | stmt ';' list
  {
    if ($3 == NULL)
    $$ = $1;
    else
    $$ = newast('L', $1, $3);
  }
;

exp
  : exp CMP exp                   { $$ = newcmp($2, $1, $3); }
  | exp '+' exp                   { $$ = newast('+', $1,$3); }
  | exp '-' exp                   { $$ = newast('-', $1,$3); }
  | exp '*' exp                   { $$ = newast('*', $1,$3); }
  | exp '/' exp                   { $$ = newast('/', $1,$3); }
  | '%' exp                       { $$ = newast('%', $2, NULL); }
  | '(' exp ')'                   { $$ = $2; }
  | '-' exp %prec UMINUS          { $$ = newast('M', $2, NULL); }
  | NUMBER                        { $$ = newnum($1); }
  | NAME                          { $$ = newref($1); }
  | NAME '=' exp                  { $$ = newasgn($1, $3); }
  | FUNC '(' explist ')'          { $$ = newfunc($1, $3); }
  | NAME '(' explist ')'          { $$ = newcall($1, $3); }
  | '{' '}'                       { $$ = newast(SETAST, NULL, NULL); }
  | '{' explist '}'               { $$ = newast(SETAST, $2, NULL); }
  | '[' ']'                       { $$ = newast(LISTAST, NULL, NULL); }
  | '[' explist ']'               { $$ = newast(LISTAST, $2, NULL); }
  | exp SETOP exp                 { $$ = newast($2, $1, $3); }
  | ELEM                          { $$ = newelem($1); }
  | exp LOGICOP exp               { $$ = newast($2, $1, $3); }
  | NOT exp                       { $$ = newast($1, $2, NULL); }
  | exp '[' exp ']'               { $$ = newast('P', $1, $3); } /* La primera posicion es 0 */
  | POP exp                       { $$ = newast(POPOP, $2, NULL); }
  | exp '#' exp                   { $$ = newast('#', $1, $3); }
;

explist
  : exp
  | exp ',' explist               { $$ = newast('L', $1, $3); }
;

symlist
  : NAME                          { $$ = newsymlist($1, NULL); }
  | NAME ',' symlist              { $$ = newsymlist($1, $3); }
;

calclist
  : /* nothing */
  | calclist stmt EOL
  {
    tData v = eval($2);
    printf("= ");
    PrintDT(v);
    printf("\n> ");
    treefree($2);
    FreeDT(&v);
  }
  | calclist LET NAME '(' symlist ')' '=' list EOL
  {
    dodef($3, $5, $8);
    printf("Defined %s\n> ", $3->name);
  }
  | calclist error EOL
  {
    yyerrok; printf("> ");
  }
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
