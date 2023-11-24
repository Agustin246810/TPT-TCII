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

ast newast(int nodetype, ast l, ast r)
{
  ast a = malloc(sizeof(struct tAst));

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

ast newnum(double d)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = CONSTANTAST;
  a->number = d;
  return a;
}

ast newcmp(int cmptype, ast l, ast r)
{
  ast a = malloc(sizeof(struct tAst));

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

ast newfunc(int functype, ast l)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = FNCALLAST;
  a->l = l;
  a->functype = functype;
  return a;
}

ast newcall(struct symbol *s, ast l)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = UFNCALLAST;
  a->l = l;
  a->sym = s; // sym por ser ufncall
  return a;
}

ast newref(struct symbol *s)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = SYMREFAST;
  a->s = s;
  return a;
}

ast newasgn(struct symbol *s, ast v)
{
  ast a = malloc(sizeof(struct tAst));
  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = ASSIGNMENTAST;
  a->s = s;
  a->v = v;
  return a;
}

ast newflow(int nodetype, ast cond, ast tl, ast el)
{
  ast a = malloc(sizeof(struct tAst));

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

ast newforeach(struct symbol *sym, ast exp, ast tl)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = FOREACHAST;
  a->symvar = sym;
  a->l = exp;
  a->r = tl;

  return a;
}

/* free a tree of ASTs */
void treefree(ast a)
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
  case GREATEROREQUALOP:
  case LESSEROREQUALOP:
  case 'L':
  case POSITIONEDELEM:
  case ISCOINTAINED:
  case ANDOP:
  case OROP:
  case UNIONOP:
  case DIFFOP:
  case INTERSOP:
  case EXCHANGEOP:
  case FOREACHAST:
    treefree(a->r);
  /* one subtree */
  // case ABSVALUEAST:
  case UMINUSOP:
  case UFNCALLAST:
  case FNCALLAST:
  case NOTOP:
  case SETAST:
  case LISTAST:
  case POPOP:
    treefree(a->l);
  /* no subtree */
  case CONSTANTAST:
  case SYMREFAST:
    break;
  case ASSIGNMENTAST:
    free(a->v);
    break;
    /* up to three subtrees */
  case IFAST:
  case WHILEAST:
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

struct symlist *newsymlist(struct symbol *sym, struct symlist *next, int isRef)
{
  struct symlist *sl = malloc(sizeof(struct symlist));

  if (!sl)
  {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  sl->isref = isRef;
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

static tData callbuiltin(ast);
static tData calluser(ast);

tData eval(ast a)
{
  ast auxAST; // variable auxiliar para recorrer un subarbol
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
  case CONSTANTAST:
    v = CreateDoubleDT(a->number);
    break;
  /* name reference */
  case SYMREFAST:
    v = CopyDT(a->s->value);
    break;
  /* assignment */
  case ASSIGNMENTAST:
    l = eval(a->v);

    FreeDT(&a->s->value);
    a->s->value = CopyDT(l);
    v = CopyDT(l);

    FreeDT(&l);
    break;

  /*Assigment Exchange*/
  case EXCHANGEOP:
    l = eval(a->l);
    r = eval(a->r);
    auxDT = a->symvar->value;

    if (TypeDT(l) != DOUBLE)
    {
      printf("Error: position is not a number.");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) != floor(ValueDT(l)))
    {
      printf("Error: position is not an integer.");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) < 0)
    {
      printf("Error: negative position.");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) >= SizeL(auxDT))
    {
      printf("Error: position overflow.");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    ExchangeL(auxDT, r, (int)ValueDT(l) + 1);

    v = CopyDT(ElemDT(auxDT, (int)ValueDT(l) + 1));

    FreeDT(&l);
    FreeDT(&r);

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
  // TODO: Agregar el pertenece.
  case ISCOINTAINED:
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

    if (a->l->nodetype == SYMREFAST)
      l = a->l->s->value;
    else
      l = eval(a->l);

    if (TypeDT(l) != LIST)
    {
      printf("Invalid POP: the expression is not a list.\n");
      if (a->l->nodetype != SYMREFAST)
        FreeDT(&l);
      break;
    }

    if (SizeL(l) == 0)
    {
      printf("Invalid POP: empty list.\n");
      if (a->l->nodetype != SYMREFAST)
        FreeDT(&l);
      break;
    }

    v = Pop(&l);

    if (a->l->nodetype != SYMREFAST)
      FreeDT(&l);

    break;
  /* Positioned Element */
  case POSITIONEDELEM:

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
    v = CopyDT(ElemDT(l, ((int)ValueDT(r)) + 1));

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
      printf("Subtraction error: at least one of the operands is not a number.\n");

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

  case UMINUSOP:
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
  case GREATEROREQUALOP:
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
  case IFAST: // TODO: comprobar tipos
    if (ValueDT(eval(a->cond)) != 0)
    {
      if (a->tl)
      {
        l = eval(a->tl);
        v = CopyDT(l);
        FreeDT(&l);
      }
    }
    else
    {
      if (a->el)
      {
        l = eval(a->el);
        v = CopyDT(l);
        FreeDT(&l);
      }
    }
    break;
    /* while/do */
  case WHILEAST:             // TODO: comprobar tipos
    v = CreateDoubleDT(0.0); /* a default value */

    if (a->tl)
    {
      r = eval(a->cond);

      while (ValueDT(r) != 0)
      {
        FreeDT(&v);

        l = eval(a->tl);
        v = CopyDT(l);
        FreeDT(&l);

        FreeDT(&r);
        r = eval(a->cond);
      }

      FreeDT(&r);
    }
    break; /* value of last statement is value of while/do */

  case FOREACHAST:
    v = CreateDoubleDT(0.0); // valor por defecto

    l = eval(a->l);

    if (TypeDT(l) != SET && TypeDT(l) != LIST)
    {
      printf("FOREACH error: argument is not a set or a list.\n");

      FreeDT(&l);

      break;
    }

    if (a->r)
    {
      if (TypeDT(l) == SET)
      {
        for (int pos = 1; pos <= Cardinal(l); pos++)
        {
          FreeDT(&(a->symvar->value));
          a->symvar->value = CopyDT(ElemDT(l, pos));

          r = eval(a->r);
          FreeDT(&r);

          FreeDT(&l);
          l = eval(a->l);

          if (TypeDT(l) != SET)
          {
            break;
          }
        }
      }
      else
      {
        for (int pos = 1; pos <= SizeL(l); pos++)
        {
          FreeDT(&(a->symvar->value));
          a->symvar->value = CopyDT(ElemDT(l, pos));

          r = eval(a->r);
          FreeDT(&r);

          FreeDT(&l);
          l = eval(a->l);

          if (TypeDT(l) != LIST)
          {
            break;
          }
        }
      }
    }

    FreeDT(&l);

    break;

  /* list of statements */
  case 'L':
    l = eval(a->l);

    v = eval(a->r);

    FreeDT(&l);
    break;

  case FNCALLAST:
    l = callbuiltin(a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  case UFNCALLAST:
    l = calluser(a);
    v = CopyDT(l);
    FreeDT(&l);
    break;
  default:
    printf("internal error: bad node %c\n", a->nodetype);
  }

  return v ? v : CreateDoubleDT(0.0);
}

static tData callbuiltin(ast f)
{
  enum bifs functype = f->functype;

  tData aux = eval(f->l);

  if (TypeDT(aux) != DOUBLE) // Verificación del tipo de dato del parametro,
  {                          // de momento solo admite DOUBLE, TODO: completar tipos
    yyerror("Wrong type of parameter: %d", TypeDT(aux));

    FreeDT(&aux);

    return CreateDoubleDT(0.0);
  }

  double v = ValueDT(aux);
  FreeDT(&aux);

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
  case B_abs:
    return CreateDoubleDT(fabs(v));
  default:
    yyerror("Unknown built-in function %d", functype);
    return CreateDoubleDT(0.0);
  }
}

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, ast func)
{
  if (name->syms)
    symlistfree(name->syms);
  if (name->func)
    treefree(name->func);
  name->syms = syms;
  name->func = func;
}

static tData calluser(ast f)
{
  // sym por ser ufncall
  struct symbol *fn = f->sym; /* function name */
  struct symlist *sl;         /* dummy arguments */
  ast args = f->l;            /* actual arguments */
  double *oldval, *newval;    /* saved arg values */
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

ast newelem(char *c)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = ELEMAST;
  a->c = strdup(c);
  return a;
}

ast newexchange(struct symbol *s, ast l, ast r)
{
  ast a = malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = EXCHANGEOP;
  a->l = l;
  a->r = r;
  a->symvar = s;
  return a;
}
