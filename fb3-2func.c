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

// Devuelve la cantidad de elementos de una symlist
int _elementCountSL(struct symlist *sl);
// Devuelve la cantidad de elementos de una explist (ast)
int _elementCountEL(ast explist);

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = SYMREFAST;
  a->s = s;
  return a;
}

ast newmultiasgn(struct symlist *sl, ast v)
{
  ast a = (ast)malloc(sizeof(struct tAst));
  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = MULTIASSIGNMENTAST;
  a->sl = sl;
  a->v = v;
  return a;
}

ast newasgn(struct symbol *s, ast v)
{
  ast a = (ast)malloc(sizeof(struct tAst));
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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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

ast newaliasing(struct symbol *dest, struct symbol *src)
{
  ast a = (ast)malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = ALIASINGAST;
  a->adest = dest;
  a->asrc = src;
  return a;
}

ast newmultiname(struct symbol *s, struct symlist *sl)
{
  ast a = (ast)malloc(sizeof(struct tAst));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = MULTINAMEAST;
  a->s = s;
  a->sl = sl;
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
  case ISINCLUDED:
  case PUSHOP:
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
  case ALIASINGAST:
    break;
  case ASSIGNMENTAST:
    treefree(a->v);
    break;
  case MULTIASSIGNMENTAST:
    symlistfree(a->sl);
    treefree(a->v);
    break;
  case MULTINAMEAST:
    symlistfree(a->sl);
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

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
  struct symlist *sl = (struct symlist *)malloc(sizeof(struct symlist));

  if (!sl)
  {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  // sl->isref = isRef;
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
  tData result = NULL;
  tData l, r, auxDT; // Auxiliares para liberar memoria
  struct symlist *auxSL;
  int i = 0;
  if (!a)
  {
    yyerror("internal error, null eval");
    return NULL;
  }
  switch (a->nodetype)
  {
  /* constant */
  case CONSTANTAST:
    result = CreateDoubleDT(a->number);
    break;
  /* name reference */
  case SYMREFAST:
    result = CopyDT(a->s->value);
    break;
  /* assignment */
  case MULTIASSIGNMENTAST:
    // En caso de que tengan distinta cantidad de elementos
    if (_elementCountSL(a->sl) != _elementCountEL(a->v))
    { // La cantida de elementos siempre va a ser mayor a 0 por bison
      if (_elementCountSL(a->sl) > _elementCountEL(a->v))
      {
        printf("Assignment error: there are more symbols than expressions.\n");
      }
      else
      {
        printf("Assignment error: there are more expressions than symbols.\n");
      }
    }
    auxSL = a->sl;
    auxAST = a->v;

    // Se asignan todos menos el ultimo elemento
    while (auxAST->nodetype == 'L')
    {
      auxDT = eval(auxAST->l);

      Overwrite(auxDT, auxSL->sym->value);

      FreeDT(&auxDT);

      auxSL = auxSL->next;
      auxAST = auxAST->r;
    }

    // Se asigna el ultimo elemento
    auxDT = eval(auxAST);

    Overwrite(auxDT, auxSL->sym->value);

    FreeDT(&auxDT);
    break;

  case ASSIGNMENTAST:
    result = eval(a->v);

    Overwrite(result, a->s->value);

    break;

  case MULTINAMEAST:

    auxDT = CopyDT(a->s->value);

    if (TypeDT(auxDT) != SET && TypeDT(auxDT) != LIST) // Comprobamos que el symbol contenga una lista o conjunto
    {
      printf("Multiname error: type must be SET or LIST.\n");

      FreeDT(&auxDT);
      break;
    }

    // Comprobamos que el tamaño de la estructura coincida con la cantidad de simbolos
    if (TypeDT(auxDT) == SET)
    {
      if (Cardinal(auxDT) != _elementCountSL(a->sl))
      {
        printf("Multiname error: the amount of names and the cardinal of the set must be the same.\n");

        FreeDT(&auxDT);
        break;
      }
    }
    else
    {
      if (SizeL(auxDT) != _elementCountSL(a->sl))
      {
        printf("Multiname error: the amount of names and the size of the list must be the same.\n");

        FreeDT(&auxDT);
        break;
      }
    }

    // Comprobamos que el simbolo a la izquierda no se llame a la derecha
    auxSL = a->sl;

    while (auxSL)
    {
      if (a->s == auxSL->sym)
      {
        break;
      }

      auxSL = auxSL->next;
    }

    if (auxSL) // Significa que encontro el mismo nombre en la izquierda y la derecha
    {
      printf("Multiname error: can't use the same name in the left and in the right.\n");

      FreeDT(&auxDT);
      break;
    }

    // Asignaciones
    auxSL = a->sl;
    i = 1;

    while (auxSL)
    {
      auxSL->sym->value = CopyDT(ElemDT(auxDT, i));

      auxSL = auxSL->next;
      i++;
    }

    result = auxDT;

    break;

  case ALIASINGAST:
    // Liberacion de memoria
    for (i = 0; i < NHASH - 1; i++)
    {
      if ((&symtab[i])->value == a->adest->value && (&symtab[i]) != a->adest)
      {
        break;
      }
    }

    if (i == NHASH - 1 && (&symtab[i])->value != a->adest->value)
    {
      FreeDT(&(a->adest->value));
    }

    a->adest->value = a->asrc->value;

    result = CopyDT(a->adest->value);

    break;

  /*Assigment Exchange*/
  case EXCHANGEOP:
    l = eval(a->l);
    r = eval(a->r);
    auxDT = a->symvar->value;

    if (TypeDT(l) != DOUBLE)
    {
      printf("Exchange error: position is not a number.\n");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) != floor(ValueDT(l)))
    {
      printf("Exchange error: position is not an integer.\n");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) < 0)
    {
      printf("Exchange error: negative position.\n");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    if (ValueDT(l) >= SizeL(auxDT))
    {
      printf("Exchange error: position overflow.\n");

      FreeDT(&l);
      FreeDT(&r);
      break;
    }

    ExchangeL(auxDT, r, (int)ValueDT(l) + 1);

    result = CopyDT(ElemDT(auxDT, (int)ValueDT(l) + 1));

    FreeDT(&l);
    FreeDT(&r);

    break;

  /* Elem */
  case ELEMAST:
    result = CreateDT(a->c);
    break;

  /* Literal Set */
  case SETAST:
    result = CreateDT("{}"); // Se crea el conjunto originalmente vacio

    if (a->l) // En caso de contener expresiones, se agregan una por una
    {
      auxAST = a->l;

      while (auxAST->nodetype == 'L') // Se agregan las expresiones una por una
      {
        l = eval(auxAST->l);
        r = CreateDT3(SET, l);

        auxDT = Union(result, r);

        FreeDT(&l);
        FreeDT(&r);
        FreeDT(&result);

        result = auxDT;
        auxDT = NULL;

        auxAST = auxAST->r;
      }

      // Se agrega el ultimo elemento que esta en el hijo derecho de la ultima 'L'
      l = eval(auxAST);
      r = CreateDT3(SET, l);

      auxDT = Union(result, r);

      FreeDT(&l);
      FreeDT(&r);
      FreeDT(&result);

      result = auxDT;
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

    result = Union(l, r);

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

    result = Inter(l, r);

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

    result = Diff(l, r);

    FreeDT(&l);
    FreeDT(&r);

    break;

  case ISINCLUDED:

    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(r) != SET && TypeDT(r) != LIST)
    {
      printf("IsIncluded error: the second argument must be a set or list.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    if (TypeDT(r) == SET) // En caso de ser conjunto
    {
      result = CreateDoubleDT(In(r, l) ? 1 : 0);
    }
    else // En caso de ser lista
    {
      for (int i = 1; i <= SizeL(r); i++)
      {
        if (CompareDT(l, ElemDT(r, i)))
        {
          result = CreateDoubleDT(1);

          break;
        } // En caso de salir del for sin pasar por el if va a quedar con valor 0 en el return
      }
    }

    break;

  case ISCOINTAINED:
    l = eval(a->l);
    r = eval(a->r);

    if (TypeDT(l) != SET || TypeDT(r) != SET)
    {
      printf("IsContained error: at least one of the operands is not a set.\n");

      FreeDT(&l);
      FreeDT(&r);

      break;
    }

    result = CreateDoubleDT(IsContained(l, r) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);

    break;

  /* Literal List */
  case LISTAST:
    result = CreateDT("[]"); // Se crea la lista originalmente vacia

    if (a->l) // En caso de contener expresiones, se agregan una por una
    {
      auxAST = a->l;

      while (auxAST->nodetype == 'L') // Se agregan las expresiones una por una
      {
        l = eval(auxAST->l);

        Push(result, l);

        FreeDT(&l);

        auxAST = auxAST->r;
      }

      // Se agrega el ultimo elemento que esta en el hijo derecho de la ultima 'L'
      l = eval(auxAST);

      Push(result, l);

      FreeDT(&l);
    }
    break;

  /*POP*/
  case POPOP:

    if (a->l->nodetype == SYMREFAST) // En caso de ser un symbol
      l = a->l->s->value;
    else // En caso de ser un literal
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

    result = Pop(&l);

    if (a->l->nodetype != SYMREFAST)
      FreeDT(&l);

    break;

  case PUSHOP:
    if (a->r->nodetype == SYMREFAST) // En caso de ser un symbol
      r = a->r->s->value;
    else // En caso de ser un literal
      r = eval(a->r);

    if (TypeDT(r) != LIST)
    {
      printf("Invalid PUSH: the right expression is not a list.\n");
      if (a->r->nodetype != SYMREFAST)
        FreeDT(&r);
      break;
    }

    l = eval(a->l);

    Push(r, l);

    result = CopyDT(r);

    FreeDT(&l);

    if (a->r->nodetype != SYMREFAST)
      FreeDT(&r);

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
    result = CopyDT(ElemDT(l, ((int)ValueDT(r)) + 1));

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

    result = CreateDoubleDT(ValueDT(l) + ValueDT(r));

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

    result = CreateDoubleDT(ValueDT(l) - ValueDT(r));

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

    result = CreateDoubleDT(ValueDT(l) * ValueDT(r));

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

    result = CreateDoubleDT(ValueDT(l) / ValueDT(r));

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

    result = CreateDoubleDT(-ValueDT(l));

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

    result = CreateDoubleDT((ValueDT(l) > ValueDT(r)) ? 1 : 0);

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

    result = CreateDoubleDT((ValueDT(l) < ValueDT(r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case NOTEQUALOP: // Esta operacion funciona para conjuntos, listas, numeros y elementos.
    // v = (eval(a->l) != eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    // v = CreateDoubleDT((ValueDT(l) != ValueDT(r)) ? 1 : 0);
    result = CreateDoubleDT((!CompareDT(l, r)) ? 1 : 0);

    FreeDT(&l);
    FreeDT(&r);
    break;
  case ISEQUALOP: // Esta operacion funciona para conjuntos, listas, numeros y elementos.
    // v = (eval(a->l) == eval(a->r)) ? 1 : 0;
    l = eval(a->l);
    r = eval(a->r);

    // v = CreateDoubleDT((ValueDT(l) == ValueDT(r)) ? 1 : 0);
    result = CreateDoubleDT((CompareDT(l, r)) ? 1 : 0);

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

    result = CreateDoubleDT((ValueDT(l) >= ValueDT(r)) ? 1 : 0);

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

    result = CreateDoubleDT((ValueDT(l) <= ValueDT(r)) ? 1 : 0);

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

    result = CreateDoubleDT((ValueDT(l) && ValueDT(r)) ? 1 : 0);

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

    result = CreateDoubleDT((ValueDT(l) || ValueDT(r)) ? 1 : 0);

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

    result = CreateDoubleDT(((!ValueDT(l)) ? 1 : 0));

    FreeDT(&l);
    break;

    /* control flow */
    /* null expressions allowed in the grammar, so check for them */
    /* if/then/else */
  case IFAST: // TODO: comprobar tipos
    auxDT = eval(a->cond);

    if (TypeDT(auxDT) != DOUBLE)
    {
      printf("If error: the condition must be a number.\n");

      FreeDT(&auxDT);

      break;
    }

    if (ValueDT(auxDT) != 0)
    {
      if (a->tl)
      {
        result = eval(a->tl);
      }
    }
    else
    {
      if (a->el)
      {
        result = eval(a->el);
      }
    }

    FreeDT(&auxDT);

    break;
    /* while/do */
  case WHILEAST:

    r = eval(a->cond);

    if (TypeDT(r) != DOUBLE)
    {
      printf("While error: the condition must be a number.\n");

      break;
    }

    if (a->tl)
    {
      while (ValueDT(r) != 0)
      {
        FreeDT(&result); // Se libera el anterior

        result = eval(a->tl);

        FreeDT(&r);
        r = eval(a->cond);
      }
    }

    FreeDT(&r);

    break; /* value of last statement is value of while/do */

  case FOREACHAST:
    result = CreateDoubleDT(0.0); // valor por defecto

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

    result = eval(a->r);

    FreeDT(&l);
    break;

  case FNCALLAST:
    l = callbuiltin(a);
    result = CopyDT(l);
    FreeDT(&l);
    break;
  case UFNCALLAST:
    l = calluser(a);
    result = CopyDT(l);
    FreeDT(&l);
    break;
  default:
    printf("internal error: bad node %c\n", a->nodetype);
  }

  return result ? result : CreateDoubleDT(0.0);
}

static tData callbuiltin(ast f)
{
  enum bifs functype = f->functype;

  tData auxDT = eval(f->l);

  double result = 0;

  switch (functype)
  {
  case B_sqrt:
    if (TypeDT(auxDT) != DOUBLE)
    {
      yyerror("Wrong type of parameter: %d", TypeDT(auxDT));

      FreeDT(&auxDT);
      return CreateDoubleDT(0.0);
    }

    result = sqrt(ValueDT(auxDT));

    FreeDT(&auxDT);

    return CreateDoubleDT(result);

  case B_exp:
    if (TypeDT(auxDT) != DOUBLE)
    {
      yyerror("Wrong type of parameter: %d", TypeDT(auxDT));

      FreeDT(&auxDT);
      return CreateDoubleDT(0.0);
    }

    result = exp(ValueDT(auxDT));

    FreeDT(&auxDT);

    return CreateDoubleDT(result);

  case B_log:
    if (TypeDT(auxDT) != DOUBLE)
    {
      yyerror("Wrong type of parameter: %d", TypeDT(auxDT));

      FreeDT(&auxDT);
      return CreateDoubleDT(0.0);
    }

    result = log(ValueDT(auxDT));

    FreeDT(&auxDT);

    return CreateDoubleDT(result);

  case B_print: // Aqui no importa el tipo
    printf("= ");
    PrintDT(auxDT);
    printf("\n");

    return auxDT; // En vez de liberarlo, lo devolvemos

  case B_abs:
    if (TypeDT(auxDT) != DOUBLE)
    {
      yyerror("Wrong type of parameter: %d", TypeDT(auxDT));

      FreeDT(&auxDT);
      return CreateDoubleDT(0.0);
    }

    result = fabs(ValueDT(auxDT));

    FreeDT(&auxDT);

    return CreateDoubleDT(result);

  case B_size:
    if (TypeDT(auxDT) != SET && TypeDT(auxDT) != LIST)
    {
      yyerror("The parameter must be a set or list.");

      FreeDT(&auxDT);
      return CreateDoubleDT(0.0);
    }

    if (TypeDT(auxDT) == SET)
    {
      result = Cardinal(auxDT);
    }
    else
    {
      result = SizeL(auxDT);
    }

    FreeDT(&auxDT);

    return CreateDoubleDT(result);

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
  ast a = (ast)malloc(sizeof(struct tAst));

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
  ast a = (ast)malloc(sizeof(struct tAst));

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

// Devuelve la cantidad de elementos de una symlist
int _elementCountSL(struct symlist *sl)
{
  if (!sl)
  {
    return 0;
  }

  return 1 + _elementCountSL(sl->next);
}

// Devuelve la cantidad de elementos de una explist
int _elementCountEL(ast explist)
{
  if (explist->nodetype != 'L')
  {
    return 1; // Ultimo elemento a la derecha
  }

  return 1 + _elementCountEL(explist->r);
}
