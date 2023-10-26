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
  struct ast *a = malloc(sizeof(struct ast));

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
  a->nodetype = cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newfunc(int functype, struct ast *l)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return a;
}

struct ast *newref(struct symbol *s)
{
  struct ast *a = malloc(sizeof(struct ast));

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
  struct ast *a = malloc(sizeof(struct ast));
  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return a;
}

/* free a tree of ASTs */
void treefree(struct ast *a)
{
  if (!a) // Agregado para que no de error al ingresar NULL
  {       // al liberar conjunto o lista vacia
    return;
  }

  switch (a->nodetype)
  {
  /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case GREATEROP:
  case LESSEROP:
  case NOTEQUALOP:
  case ISEQUALOP:
  case GEATEROREQUALOP:
  case LESSEROREQUALOP:
  case 'L':
  case 'P':
  case '#':
  case ANDOP:
  case OROP:
  case UNIONOP:
  case DIFFOP:
  case INTERSOP:
    treefree(a->r);
  /* one subtree */
  case '%':
  case 'M':
  case 'C':
  case 'F':
  case NOTOP:
  case SETAST:
  case LISTAST:
  case POPOP:
    treefree(a->l);
  /* no subtree */
  case 'K':
  case 'N':
    break;
  case '=':
    free(a->v);
    break;
    /* up to three subtrees */
  case 'I':
  case 'W':
    free(a->cond);
    if (a->tl)
      treefree(a->tl);
    if (a->el)
      treefree(a->el);
    break;
  case ELEMAST:
    free(a->c);
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

static tData callbuiltin(struct ast *);
static tData calluser(struct ast *);

tData eval(struct ast *a)
{
  struct ast *auxAST; // variable auxiliar para recorrer un subarbol
  tData v = NULL;
  tData l, r, auxDT; // Auxiliares para liberar memoria
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
    v = CreateDoubleDT(a->number);
    break;
  /* name reference */
  case 'N':
    // v = ((struct symref *)a)->s->value;
    v = CopyDT(a->s->value);
    break;
  /* assignment */
  case '=':
    // v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v);
    l = eval(a->v);

    a->s->value = CopyDT(l);
    v = CopyDT(l);

    FreeDT(&l);
    break;

  /* Elem */
  case ELEMAST:
    v = CreateDT(a->c);
    break;

  /* Literal Set */
  case SETAST:
    v = CreateDT("{}"); // Se crea el conjunto originalmente vacio

    if (a->l) // En caso de contener expresiones, se agregan una por una
    {
      auxAST = a->l;

      while (auxAST->nodetype == 'L') // Se agregan las expresiones una por una
      {
        l = eval(auxAST->l);
        r = CreateDT3(SET, l);

        // printf("\n");
        // PrintDT(v);

        auxDT = Union(v, r);

        FreeDT(&l);
        FreeDT(&r);
        FreeDT(&v);

        v = auxDT;
        auxDT = NULL;

        auxAST = auxAST->r;
      }

      // Se agrega el ultimo elemento que esta en el hijo derecho de la ultima 'L'
      l = eval(auxAST);
      r = CreateDT3(SET, l);

      auxDT = Union(v, r);

      FreeDT(&l);
      FreeDT(&r);
      FreeDT(&v);

      v = auxDT;
      auxDT = NULL;
    }
    break;

  /* Set Operations */
  case UNIONOP:

    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET || TypeDT(r) != SET)
    {
      printf("Union error: at least one of the operands is not a set.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = Union(l, r);

    FreeDT(&l);
    FreeDT(&r);

    break;
  case INTERSOP:

    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET || TypeDT(r) != SET)
    {
      printf("Intersection error: at least one of the operands is not a set.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = Inter(l, r);

    FreeDT(&l);
    FreeDT(&r);

    break;
  case DIFFOP:

    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET || TypeDT(r) != SET)
    {
      printf("Difference error: at least one of the operands is not a set.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = Diff(l, r);

    FreeDT(&l);
    FreeDT(&r);

    break;

  case '#':
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET || TypeDT(r) != SET)
    {
      printf("IsContainer error: at least one of the operands is not a set.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT(IsContained(l, r) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);

    break;

  /* Literal List */
  case LISTAST:
    v = CreateDT("[]"); // Se crea la lista originalmente vacia

    if (a->l) // En caso de contener expresiones, se agregan una por una
    {
      auxAST = a->l;

      while (auxAST->nodetype == 'L') // Se agregan las expresiones una por una
      {
        l = eval(auxAST->l);

        Push(v, l);

        FreeDT(&l);

        auxAST = auxAST->r;
      }

      // Se agrega el ultimo elemento que esta en el hijo derecho de la ultima 'L'
      l = eval(auxAST);

      Push(v, l);

      FreeDT(&l);
    }
    break;

  /*POP*/
  case POPOP:

    if (a->l->nodetype == 'N')
      l = a->l->s->value;
    else
      l = eval(a->l);

    if (TypeDT(l) != LIST)
    {
      printf("Invalid POP: the expression is not a list.\n");
      if (a->l->nodetype != 'N')
        FreeDT(&l);
      break;
    }

    if (SizeL(l) == 0)
    {
      printf("Invalid POP: empty list.\n");
      if (a->l->nodetype != 'N')
        FreeDT(&l);
      break;
    }

    v = Pop(&l);

    if (a->l->nodetype != 'N')
      FreeDT(&l);

    break;
  /* Positioned Element */
  case 'P':

    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET && TypeDT(l) != LIST) // Verificamos que el hijo izquierdo
    {                                          // sea una expresion valida (SET o LIST)
      printf("Position Element error: the first expression is not a SET or LIST.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    if (TypeDT(r) != DOUBLE) // Verificamos que el hijo derecho sea un numero
    {
      printf("Position Element error: the second expression is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    if (ValueDT(r) != floor(ValueDT(r))) // Verificamos que el hijo derecho sea un entero
    {
      printf("Position Element error: the second expression is a number but its not an integer.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    if (ValueDT(r) < 0) // Verificamos que no sea un negativo
    {
      printf("Position Element error: the second expression is an invalid value (lesser than 0).\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    // Verificamos que este dentro del rango del conjunto o lista
    if (TypeDT(l) == SET)
    {
      if (Cardinal(l) < ValueDT(r) + 1)
      {
        printf("Position Element error: position out of range.\n");

        FreeDT(&l);
        FreeDT(&r);

        break;
      }
    }
    else
    {
      if (SizeL(l) < ValueDT(r) + 1)
      {
        printf("Position Element error: position out of range.\n");

        FreeDT(&l);
        FreeDT(&r);

        break;
      }
    }

    v = ElemDT(l, ((int)ValueDT(r)) + 1);

    FreeDT(&l);
    FreeDT(&r);

    break;

  /* expressions */
  case '+':
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Addition error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT(ValueDT(l) + ValueDT(r));

    FreeDT(&l);
    FreeDT(&r);
    break;
  case '-':
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Substraction error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT(ValueDT(l) - ValueDT(r));

    FreeDT(&l);
    FreeDT(&r);
    break;
  case '*':
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Multiplication error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT(ValueDT(l) * ValueDT(r));

    FreeDT(&l);
    FreeDT(&r);
    break;
  case '/':
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Division error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT(ValueDT(l) / ValueDT(r));

    FreeDT(&l);
    FreeDT(&r);
    break;
  case '%':
    l = eval(a->l);

    if (TypeDT(l) != DOUBLE)
    {
      printf("Abstolute Value error: the operand is not a number.\n");

      FreeDT(&l);

      break;
    }

    v = CreateDoubleDT(fabs(ValueDT(l)));

    FreeDT(&l);
    break;
  case 'M':
    l = eval(a->l);

    if (TypeDT(l) != DOUBLE)
    {
      printf("UMinus error: the operand is not a number.\n");

      FreeDT(&l);

      break;
    }

    v = CreateDoubleDT(-ValueDT(l));

    FreeDT(&l);
    break;
  /* comparisons */
  case GREATEROP:
    // v = (eval(a->l) > eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Greater error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) > ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case LESSEROP:
    // v = (eval(a->l) < eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("Lesser error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) < ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case NOTEQUALOP: // Esta operacion funciona para conjuntos, listas, numeros y elementos.
    // v = (eval(a->l) != eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    // v = CreateDoubleDT((ValueDT(l) != ValueDT(r)) ? 1 : 0);
    v = CreateDoubleDT((!CompareDT(l, r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case ISEQUALOP: // Esta operacion funciona para conjuntos, listas, numeros y elementos.
    // v = (eval(a->l) == eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    // v = CreateDoubleDT((ValueDT(l) == ValueDT(r)) ? 1 : 0);
    v = CreateDoubleDT((CompareDT(l, r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case GEATEROREQUALOP:
    // v = (eval(a->l) >= eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("GreaterOrEqual error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) >= ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case LESSEROREQUALOP:
    // v = (eval(a->l) <= eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("LesserOrEqual error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) <= ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;

  /* logic operations */
  case ANDOP:
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("AND operation error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) && ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case OROP:
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != DOUBLE || TypeDT(r) != DOUBLE)
    {
      printf("OR operation error: at least one of the operands is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    v = CreateDoubleDT((ValueDT(l) || ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case NOTOP:
    l = eval(a->l);

    if (TypeDT(l) != DOUBLE)
    {
      printf("NOT operation error: the operand is not a number.\n");

      FreeDT(&l);

      break;
    }

    v = CreateDoubleDT(((!ValueDT(l)) ? 1 : 0));

    FreeDT(&l);
    break;

    /* control flow */
    /* null expressions allowed in the grammar, so check for them */
    /* if/then/else */
  case 'I':
    if (ValueDT(eval(a->cond)) != 0)
    {
      if (a->tl)
      {
        l = eval(a->tl);
        v = CopyDT(l);
        FreeDT(&l);
      }
      // else /* ya se contempla en el return */
      //   v = CreateDoubleDT(0.0); /* a default value */
    }
    else
    {
      if (a->el)
      {
        l = eval(a->el);
        v = CopyDT(l);
        FreeDT(&l);
      }
      // else  /* ya se contempla en el return */
      //   v = CreateDoubleDT(0.0); /* a default value */
    }
    break;
    /* while/do */
  case 'W':
    v = CreateDoubleDT(0.0); /* a default value */

    if (a->tl)
    {
      while (ValueDT(eval(a->cond)) != 0)
      {
        l = eval(a->tl);
        v = CopyDT(l);
        FreeDT(&l);
      }
    }
    break; /* value of last statement is value of while/do */

  /* list of statements */
  case 'L':
    eval(a->l);
    v = eval(a->r);
    break;
  case 'F':
    l = callbuiltin(a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  case 'C':
    l = calluser(a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  default:
    printf("internal error: bad node %c\n", a->nodetype);
  }

  return v ? v : CreateDoubleDT(0.0);
}

static tData callbuiltin(struct ast *f)
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

static tData calluser(struct ast *f)
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
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = ELEMAST;
  a->c = strdup(c);
  return (struct ast *)a;
}