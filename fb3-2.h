#include "TDataType.h"
/*
 * Declarations for a calculator fb3-1
 */

/*AGREGADO*/
#define SETAST 400
#define LISTAST 401
#define ELEMAST 402
#define DOUBLEAST 403

#define ANDOP 500
#define OROP 501
#define NOTOP 502

#define UNIONOP 600
#define INTERSOP 601
#define DIFFOP 602
#define ISCOINTAINED 603

#define POPOP 700
#define POSITIONEDELEM 701
#define EXCHANGEOP 702

#define GREATEROP 801
#define LESSEROP 802
#define NOTEQUALOP 803
#define ISEQUALOP 804
#define GREATEROREQUALOP 805
#define LESSEROREQUALOP 806
#define UMINUSOP 807
#define CONSTANTAST 808
#define SYMREFAST 809
#define IFAST 810
#define WHILEAST 811
#define FOREACHAST 812
#define ASSIGNMENTAST 813
#define UFNCALLAST 814
#define FNCALLAST 815
// #define ABSVALUEAST 815

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(char *s, ...);

typedef struct tAst *ast;

/* symbol table */
struct symbol
{ /* a variable name */
  char *name;
  tData value;
  ast func;             /* stmt for the function */
  struct symlist *syms; /* list of dummy args */
};
/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol *lookup(char *);
/* list of symbols, for an argument list */
struct symlist
{
  struct symbol *sym;
  struct symlist *next;
};
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);
/* node types
 * + - * / |
 * 0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 * M unary minus
 * L expression or statement list
 * I IF statement
 * W WHILE statement
 * N symbol ref
 * = assignment
 * S list of symbols
 * F built in function call
 * C user function call
 *
 *
 */
enum bifs
{ /* built-in functions */
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print,
  B_abs
};
/* nodes in the abstract syntax tree */
/* all have common initial nodetype */

struct tAst
{
  int nodetype;
  union
  {
    struct // para el ast, fncall y ufncall
    {
      struct tAst *l;
      union
      {
        struct
        {
          struct tAst *r;        // ast y exchangeL
          struct symbol *symvar; // solo para exchangeL y Foreach
        };
        enum bifs functype; // fncall
        struct symbol *sym; // ufncall
      };
    };

    struct // para el flow
    {
      struct tAst *cond; // condition
      struct tAst *tl;   // then branch or do list
      struct tAst *el;   // optional else branch
    };

    struct // para el symref y symasgn
    {
      struct symbol *s;
      struct tAst *v; // solo para el symasgn
    };

    double number; // para el numval
    char *c;       // para el elemast
  };
};

/* build an AST */
ast newast(int nodetype, ast l, ast r);
ast newcmp(int cmptype, ast l, ast r);
ast newfunc(int functype, ast l);
ast newcall(struct symbol *s, ast l);
ast newref(struct symbol *s);
ast newasgn(struct symbol *s, ast v);
ast newnum(double d);
ast newflow(int nodetype, ast cond, ast tl, ast el);
ast newelem(char *c);
ast newexchange(struct symbol *s, ast l, ast r);
ast newforeach(struct symbol *sym, ast exp, ast tl);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, ast stmts);
/* evaluate an AST */
tData eval(ast);
/* delete and free an AST */
void treefree(ast);
/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(char *s, ...);
