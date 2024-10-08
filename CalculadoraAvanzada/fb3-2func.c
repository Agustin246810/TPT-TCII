/*
 * helper functions for fb3-2
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "fb3-2.h"
#include "TDataType.h"
/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;
  while (c = *sym++)
    hash = hash * 9 ^ c;
  return hash;
}

struct symbol *lookup(char *sym)
{
  struct symbol *sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH; /* how many have we looked at */
  while (--scount >= 0)
  {
    if (sp->name && !strcmp(sp->name, sym))
    {
      return sp;
    }
    if (!sp->name)
    { /* new entry */
      sp->name = strdup(sym);
      sp->value = CreateDoubleDT(0.0);
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }
    if (++sp >= symtab + NHASH)
      sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */
}

struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newnum(double d)
{
  struct numval *a = malloc(sizeof(struct numval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = d;
  return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newlogicop(int logicOpType, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = logicOpType;
  a->l = l;
  a->r = r;

  return a;
}

struct ast *newfunc(int functype, struct ast *l)
{
  struct fncall *a = malloc(sizeof(struct fncall));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
  struct ufncall *a = malloc(sizeof(struct ufncall));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
  struct symref *a = malloc(sizeof(struct symref));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));
  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
  struct flow *a = malloc(sizeof(struct flow));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return (struct ast *)a;
}

/* free a tree of ASTs */
void treefree(struct ast *a)
{
  switch (a->nodetype)
  {
  /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case 'L':
  case ANDOP:
  case OROP:
    treefree(a->r);
  /* one subtree */
  case '|':
  case 'M':
  case 'C':
  case 'F':
  case NOTOP:
    treefree(a->l);
  /* no subtree */
  case 'K':
  case 'N':
    break;
  case '=':
    free(((struct symasgn *)a)->v);
    break;
    /* up to three subtrees */
  case 'I':
  case 'W':
    free(((struct flow *)a)->cond);
    if (((struct flow *)a)->tl)
      treefree(((struct flow *)a)->tl);
    if (((struct flow *)a)->el)
      treefree(((struct flow *)a)->el);
    break;
  default:
    printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
  struct symlist *sl = malloc(sizeof(struct symlist));

  if (!sl)
  {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  return sl;
}

/* free a list of symbols */
void symlistfree(struct symlist *sl)
{
  struct symlist *nsl;
  while (sl)
  {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}

static Tree callbuiltin(struct fncall *);
static Tree calluser(struct ufncall *);

Tree eval(struct ast *a)
{
  // TODO: verificar los tipos de datos.
  Tree v = CreateDoubleDT(0.0);
  Tree l, r; // Auxiliares para liberar memoria
  if (!a)
  {
    yyerror("internal error, null eval");
    return NULL;
  }
  switch (a->nodetype)
  {
  /* constant */
  case 'K':
    // v = ((struct numval *)a)->number;
    v = CreateDoubleDT(((struct numval *)a)->number);
    break;
  /* name reference */
  case 'N':
    // v = ((struct symref *)a)->s->value;
    v = CopyDT(((struct symref *)a)->s->value);
    break;
  /* assignment */
  case '=':
    // v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v);
    ((struct symasgn *)a)->s->value = CopyDT(eval(((struct symasgn *)a)->v));
    v = CopyDT(((struct symasgn *)a)->s->value);
    break;
  /* expressions */
  case '+':
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT(ValueDT(l) + ValueDT(r));
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '-':
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT(ValueDT(l) - ValueDT(r));
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '*':
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT(ValueDT(l) * ValueDT(r));
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '/':
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT(ValueDT(l) / ValueDT(r));
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '|':
    l = eval(a->l);
    v = CreateDoubleDT(fabs(ValueDT(l)));
    FreeDT(&l);
    break;
  case 'M':
    l = eval(a->l);
    v = CreateDoubleDT(-ValueDT(l));
    FreeDT(&l);
    break;
  /* comparisons */
  case '1':
    // v = (eval(a->l) > eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) > ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '2':
    // v = (eval(a->l) < eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) < ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '3':
    // v = (eval(a->l) != eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) != ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '4':
    // v = (eval(a->l) == eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) == ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '5':
    // v = (eval(a->l) >= eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) >= ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case '6':
    // v = (eval(a->l) <= eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) <= ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  /* logic operations */
  case ANDOP:
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) && ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case OROP:
    l = eval(a->l);
    r = eval(a->r);
    v = CreateDoubleDT((ValueDT(l) || ValueDT(r)) ? 1 : 0);
    FreeDT(&l);
    FreeDT(&r);
    break;
  case NOTOP:
    l = eval(a->l);
    v = CreateDoubleDT(((!ValueDT(l)) ? 1 : 0));
    FreeDT(&l);
    break;

    /* control flow */
    /* null expressions allowed in the grammar, so check for them */
    /* if/then/else */
  case 'I':
    if (ValueDT(eval(((struct flow *)a)->cond)) != 0)
    {
      if (((struct flow *)a)->tl)
      {
        l = eval(((struct flow *)a)->tl);
        v = CopyDT(l);
        FreeDT(&l);
      }
      else
        v = CreateDoubleDT(0.0); /* a default value */
    }
    else
    {
      if (((struct flow *)a)->el)
      {
        l = eval(((struct flow *)a)->el);
        v = CopyDT(l);
        FreeDT(&l);
      }
      else
        v = CreateDoubleDT(0.0); /* a default value */
    }
    break;
    /* while/do */
  case 'W':
    v = CreateDoubleDT(0.0); /* a default value */

    if (((struct flow *)a)->tl)
    {
      while (ValueDT(eval(((struct flow *)a)->cond)) != 0)
      {
        l = eval(((struct flow *)a)->tl);
        v = CopyDT(l);
        FreeDT(&l);
      }
    }
    break; /* value of last statement is value of while/do */

  /* list of statements */
  case 'L':
    eval(a->l);
    // v = eval(a->r);
    v = eval(a->r);
    break;
  case 'F':
    l = callbuiltin((struct fncall *)a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  case 'C':
    l = calluser((struct ufncall *)a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  default:
    printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}
static Tree callbuiltin(struct fncall *f)
{
  // TODO: verificar tipo de dato de v

  enum bifs functype = f->functype;
  double v = ValueDT(eval(f->l));
  switch (functype)
  {
  case B_sqrt:
    return CreateDoubleDT(sqrt(v));
  case B_exp:
    return CreateDoubleDT(exp(v));
  case B_log:
    return CreateDoubleDT(log(v));
  case B_print:
    printf("= %4.4g\n", v);
    return CreateDoubleDT(v);
  default:
    yyerror("Unknown built-in function %d", functype);
    return CreateDoubleDT(0.0);
  }
}
/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
  if (name->syms)
    symlistfree(name->syms);
  if (name->func)
    treefree(name->func);
  name->syms = syms;
  name->func = func;
}

static Tree calluser(struct ufncall *f)
{
  struct symbol *fn = f->s; /* function name */
  struct symlist *sl;       /* dummy arguments */
  struct ast *args = f->l;  /* actual arguments */
  double *oldval, *newval;  /* saved arg values */
  double v;
  int nargs;
  int i;
  if (!fn->func)
  {
    yyerror("call to undefined function", fn->name);
    return CreateDoubleDT(0);
  }
  /* count the arguments */
  sl = fn->syms;
  for (nargs = 0; sl; sl = sl->next)
    nargs++;
  /* prepare to save them */
  oldval = (double *)malloc(nargs * sizeof(double));
  newval = (double *)malloc(nargs * sizeof(double));
  if (!oldval || !newval)
  {
    yyerror("Out of space in %s", fn->name);
    return CreateDoubleDT(0.0);
  }

  /* evaluate the arguments */
  for (i = 0; i < nargs; i++)
  {
    if (!args)
    {
      yyerror("too few args in call to %s", fn->name);
      free(oldval);
      free(newval);
      return CreateDoubleDT(0.0);
    }
    if (args->nodetype == 'L')
    { /* if this is a list node */
      newval[i] = ValueDT(eval(args->l));
      args = args->r;
    }
    else
    { /* if it's the end of the list */
      newval[i] = ValueDT(eval(args));
      args = NULL;
    }
  }

  /* save old values of dummies, assign new ones */
  sl = fn->syms;
  for (i = 0; i < nargs; i++)
  {
    struct symbol *s = sl->sym;
    oldval[i] = ValueDT(s->value);
    s->value = CreateDoubleDT(newval[i]);
    sl = sl->next;
  }
  free(newval);
  /* evaluate the function */
  v = ValueDT(eval(fn->func));
  /* put the real values of the dummies back */
  sl = fn->syms;
  for (i = 0; i < nargs; i++)
  {
    struct symbol *s = sl->sym;
    s->value = CreateDoubleDT(oldval[i]);
    sl = sl->next;
  }
  free(oldval);
  return CreateDoubleDT(v);
}

/*AGREGADO NEWELEM*/
struct ast *newelem(char *c)
{
  struct elem *a = malloc(sizeof(struct elem));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = ELEMAST;
  a->c = strdup(c);
  return (struct ast *)a;
}