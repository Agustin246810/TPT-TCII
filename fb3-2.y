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
%token IF ELSE WHILE LET FOREACH PUSH TO LARROW RARROW
%right '='
%left <fn> LOGICOP
%left <fn> SETOP
%left '['
%left ':'
%left <fn> CMP
%left POP
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%nonassoc <fn> NOT
%left IN
%token <c> ELEM
%token <s> NAME
%token <d> NUMBER
%token <fn> FUNC
%type <a> exp stmt list explist
%type <sl> symlist 
/* %type <sl> symlistAM */

%start calclist

%%

stmt
  : IF '(' exp ')' '{' list '}'                       { $$ = newflow(IFAST, $3, $6, NULL); }
  | IF '(' exp ')' '{' list '}' ELSE '{' list '}'     { $$ = newflow(IFAST, $3, $6, $10); }
  | WHILE '(' exp ')' '{' list '}'                    { $$ = newflow(WHILEAST, $3, $6, NULL); }
  | FOREACH NAME IN exp '{' list '}'                  { $$ = newforeach($2, $4, $6); }
  | PUSH exp TO exp                                   { $$ = newast(PUSHOP, $2, $4); }
  | symlist LARROW explist                            { $$ = newmultiasgn($1, $3); } // No permite aliasing
  | NAME RARROW symlist                               { $$ = newmultiname($1, $3); } // No permite aliasing
  | NAME '=' '%' NAME                                 { $$ = newaliasing($1, $4); } // ALIASING
  | exp                                               { $$ = $1; }
;

/*

a = 1
b = 2
b = %a       (b = 1)  (2? lo libero? {deberia})
b = 3        (a = 3)  (1? no se libera, se sobrescribe)
c = 4
d = 5
d = %c       (d = 4)  (5? lo libero? {deberia})
b = %c       (b = 4)  (3? lo libero? {no puedo} a tambien pasa a ser alias de c?)

*/


list
  : /* nothing */                                     { $$ = NULL; }
  | stmt ';' list
  {
    if ($3 == NULL)
    $$ = $1;
    else
    $$ = newast('L', $1, $3);
  }
;

exp
  : exp CMP exp                                       { $$ = newcmp($2, $1, $3); }
  | exp '+' exp                                       { $$ = newast('+', $1,$3); }
  | exp '-' exp                                       { $$ = newast('-', $1,$3); }
  | exp '*' exp                                       { $$ = newast('*', $1,$3); }
  | exp '/' exp                                       { $$ = newast('/', $1,$3); }
  | '(' exp ')'                                       { $$ = $2; }
  | '-' exp %prec UMINUS                              { $$ = newast(UMINUSOP, $2, NULL); }
  | NUMBER                                            { $$ = newnum($1); }
  | NAME                                              { $$ = newref($1); }
  | NAME '=' exp                                      { $$ = newasgn($1, $3); }
  | FUNC '(' explist ')'                              { $$ = newfunc($1, $3); }
  | NAME '(' explist ')'                              { $$ = newcall($1, $3); }
  | '{' '}'                                           { $$ = newast(SETAST, NULL, NULL); }
  | '{' explist '}'                                   { $$ = newast(SETAST, $2, NULL); }
  | '[' ']'                                           { $$ = newast(LISTAST, NULL, NULL); }
  | '[' explist ']'                                   { $$ = newast(LISTAST, $2, NULL); }
  | exp SETOP exp                                     { $$ = newast($2, $1, $3); }
  | exp IN exp                                        { $$ = newast(ISINCLUDED, $1, $3); }
  | ELEM                                              { $$ = newelem($1); }
  | exp LOGICOP exp                                   { $$ = newast($2, $1, $3); }
  | NOT exp %prec NOT                                 { $$ = newast($1, $2, NULL); }
  | exp '[' exp ']'                                   { $$ = newast(POSITIONEDELEM, $1, $3); } // La primera posicion es 0
  | POP exp                                           { $$ = newast(POPOP, $2, NULL); }
  | exp ':' exp                                       { $$ = newast(ISCOINTAINED, $1, $3); }
  | '#' NAME '[' exp ']' '=' exp                      { $$ = newexchange($2, $4, $7); }
;

explist
  : exp
  | exp ',' explist                                   { $$ = newast('L', $1, $3); }
;

symlist
  : NAME                                              { $$ = newsymlist($1, NULL); }
  | NAME ',' symlist                                  { $$ = newsymlist($1, $3); }
;

/* symlistAM
  : NAME                                              { $$ = newsymlist($1, NULL, 0); }
  | NAME ',' symlistAM                                { $$ = newsymlist($1, $3, 0); }
  | '#' NAME                                          { $$ = newsymlist($2, NULL, 1); }
  | '#' NAME ',' symlistAM                            { $$ = newsymlist($2, $4, 1); }
; */

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
