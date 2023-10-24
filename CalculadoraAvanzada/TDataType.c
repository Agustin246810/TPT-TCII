#include "TDataType.h"

// Recibe un puntero a una cadena con elementos sin corchetes ni llaves,
// devuelve en los parametros un puntero a una cadena con el primer elemento
// y quita dicho elemento de la cadena original.
void _GetElement(TString *s, TString *aux);

// Imprime recursivamente los elementos de
// un arbol omitiendo los corchetes y llaves.
void _PrintDTrec(Tree tree);

// Retorna una cadena nueva sin las llaves ni corchetes.
TString _RemoveBrackets(TString s);

// Retorna un puntero a un DataType de tipo STR con una cadena s.
Tree _CreateSingleDTStr(TString s);

// Retorna un puntero a un DataType de tipo SET con hijos nulos.
Tree _CreateSingleDTSet();

// Retorna un puntero a un DataType de tipo LIST con hijos nulos.
Tree _CreateSingleDTList();

Tree CopyDT(Tree original)
{
  Tree copy = NULL;

  if (original == NULL)
  {
    return NULL;
  }

  switch (original->nodeType)
  {
  case STR:
    return _CreateSingleDTStr(original->dataStr);

  case LIST:
    copy = _CreateSingleDTList();
    copy->data = CopyDT(original->data);
    copy->next = CopyDT(original->next);
    return copy;

  case DOUBLE:
    return CreateDoubleDT(original->value);

  default:
    copy = _CreateSingleDTSet();
    copy->data = CopyDT(original->data);
    copy->next = CopyDT(original->next);
    return copy;
  }
}

int IsContained(Tree set1, Tree set2)
{
  if (set1 == NULL) // Retorna 1 para la llamada recursiva
  {
    return 1;
  }

  if (set2 == NULL) // Este valor nunca deberia ser nulo ya que las
  {                 // llamadas se hacen asegurando que no lo sea.
    return 0;
  }

  if (set2->nodeType != SET || set1->nodeType != SET) // Asegura el tipo de dato
  {
    return 0;
  }

  if (set2->data == NULL) // En caso de comparar conjuntos vacios
  {
    return (set1->data == NULL);
  }

  if (!In(set2, set1->data)) // Verificamos si el elemento n de set2 esta en set1
  {
    return 0;
  }

  return IsContained(set1->next, set2); // Verificamos para el elemento n+1
}

int CompareDT(Tree elem1, Tree elem2)
{
  if (elem1 == NULL || elem2 == NULL)
  {
    return (elem1 == elem2);
  }
  else
  {
    if (elem1->nodeType != elem2->nodeType)
    {
      return 0;
    }

    switch (elem1->nodeType)
    {
    case STR:
      return (CompareStrings(elem1->dataStr, elem2->dataStr));

    case SET:
      return (IsContained(elem1, elem2) && IsContained(elem2, elem1));

    case LIST:
      return (CompareDT(elem1->data, elem2->data) && CompareDT(elem1->next, elem2->next));

    case DOUBLE:
      return (elem1->value == elem2->value);

    default:
      return 0;
    }
  }
}

TString _RemoveBrackets(TString s)
{
  int i = 1;
  TString aux;
  aux = (TString)malloc(sizeof(char) * 1000);
  while (s[i] != '\0')
  {
    aux[i - 1] = s[i];
    i++;
  }
  aux[i - 2] = '\0';
  aux = (TString)realloc(aux, sizeof(char) * i);

  return aux;
}

Tree _CreateSingleDTStr(TString s)
{
  Tree newDataType = (Tree)malloc(sizeof(struct DataType));

  newDataType->nodeType = STR;
  newDataType->dataStr = strdup(s);

  return newDataType;
}

Tree _CreateSingleDTSet()
{
  Tree newDataType = (Tree)malloc(sizeof(struct DataType));

  newDataType->nodeType = SET;
  newDataType->data = NULL;
  newDataType->next = NULL;

  return newDataType;
}

Tree _CreateSingleDTList()
{
  Tree newDataType = (Tree)malloc(sizeof(struct DataType));

  newDataType->nodeType = LIST;
  newDataType->data = NULL;
  newDataType->next = NULL;

  return newDataType;
}

Tree CreateDT(TString s)
{
  Tree newTree = NULL, aux = NULL, aux2 = NULL;
  TString element = NULL;

  switch (s[0])
  {
  case '{':

    s = _RemoveBrackets(s);

    newTree = _CreateSingleDTSet();

    _GetElement(&s, &element);

    if (StringLength(element) > 0)
    {
      newTree->data = CreateDT(element);

      aux = newTree;

      while (s != NULL)
      {
        _GetElement(&s, &element);

        aux2 = CreateDT(element);

        if (In(newTree, aux2))
        {
          FreeDT(&aux2);
        }
        else
        {
          aux->next = _CreateSingleDTSet();
          aux = aux->next;
          aux->data = aux2;
        }
      }
    }

    FreeString(&s);

    break;

  case '[':

    s = _RemoveBrackets(s);

    newTree = _CreateSingleDTList();

    _GetElement(&s, &element);

    if (StringLength(element) > 0)
    {
      newTree->data = CreateDT(element);

      aux = newTree;

      while (s != NULL)
      {
        _GetElement(&s, &element);
        aux->next = _CreateSingleDTList();
        aux = aux->next;
        aux->data = CreateDT(element);
      }
    }

    FreeString(&s);

    break;

  default:

    newTree = _CreateSingleDTStr(s);
  }

  return newTree;
}

void PrintDT(Tree tree)
{
  if (tree != NULL)
  {
    if (tree->nodeType == STR)
    {
      printf(StrDT(tree));
    }
    else
    {
      if (tree->nodeType == SET)
      {
        printf("{");
        _PrintDTrec(tree);
        printf("}");
      }
      else
      {
        if (tree->nodeType == DOUBLE)
        {
          printf("%4.4g", tree->value);
        }
        else
        {
          printf("[");
          _PrintDTrec(tree);
          printf("]");
        }
      }
    }
  }
}

void _GetElement(TString *s, TString *aux)
{
  TString auxString;
  int i = 0;
  int countOpenBrace = 0;
  int countOpenBracket = 0;
  int countCloseBrace = 0;
  int countCloseBracket = 0;
  int j, leave;

  *aux = (TString)malloc(sizeof(char) * 500);
  auxString = (TString)malloc(sizeof(char) * 500);

  if ((*s)[0] != '{' && (*s)[0] != '[')
  {
    while ((*s)[i] != '\0' && (*s)[i] != ',')
    {
      (*aux)[i] = (*s)[i];
      i++;
    }

    (*aux)[i] = '\0';
  }
  else
  {
    leave = 0;

    while ((*s)[i] != '\0' && !leave)
    {
      (*aux)[i] = (*s)[i];

      switch ((*s)[i])
      {
      case '{':
        countOpenBrace++;
        break;
      case '}':
        countCloseBrace++;
        break;
      case '[':
        countOpenBracket++;
        break;
      case ']':
        countCloseBracket++;
        break;
      }

      if (countCloseBracket == countOpenBracket && countCloseBrace == countOpenBrace)
      {
        leave = 1;
      }
      i++;
    }

    (*aux)[i] = '\0';
  }

  i++;

  if (i < StringLength(*s))
  {
    j = 0;

    while ((*s)[i] != '\0')
    {
      auxString[j] = (*s)[i];
      j++;
      i++;
    }

    auxString[j] = '\0';
    *s = auxString;
  }
  else
  {
    *s = NULL;
  }
}

void _PrintDTrec(Tree tree)
{
  if (tree != NULL)
  {
    if (tree->nodeType == STR)
    {
      printf(tree->dataStr);
    }
    else
    {
      if (tree->nodeType == DOUBLE)
      {
        printf("%4.4g", tree->value);
        return;
      }

      PrintDT(tree->data);
      if (tree->next != NULL)
      {
        printf(",");
        _PrintDTrec(tree->next);
      }
    }
  }
}

int SizeL(Tree L)
{
  if (L == NULL)
  {
    return -1;
  }

  if (L->nodeType != LIST)
  {
    return -1;
  }

  if (L->data == NULL)
  {
    return 0;
  }

  int count = 0;

  while (L != NULL)
  {
    count++;
    L = L->next;
  }

  return count;
}

int Cardinal(Tree S)
{

  if (S == NULL)
  {
    return -1;
  }

  if (S->nodeType != SET)
  {
    return -1;
  }

  if (S->data == NULL)
  {
    return 0;
  }

  int count = 0;

  while (S != NULL)
  {
    count++;
    S = S->next;
  }

  return count;
}

int TypeDT(Tree d)
{
  return d->nodeType;
}

TString StrDT(Tree d)
{
  if (d == NULL)
  {
    return NULL;
  }

  if (d->nodeType != STR)
  {
    return NULL;
  }

  if (d->dataStr == NULL)
  {
    return NULL;
  }

  return d->dataStr;
}

double ValueDT(Tree d)
{
  if (!d)
  {
    printf("Error, NULL point.");
    return 0.0;
  }

  if (d->nodeType != DOUBLE)
  {
    printf("Error, tipo incorrecto.");
    return 0.0;
  }

  return d->value;
}

Tree ElemDT(Tree CL, int pos)
{
  if (CL == NULL)
  {
    return NULL;
  }

  if (CL->nodeType == STR)
  {
    return NULL;
  }

  while (CL != NULL && pos > 1)
  {
    CL = CL->next;
    pos--;
  }

  if (CL == NULL)
  {
    return NULL;
  }

  return CL->data;
}

void Push(Tree L, Tree elem)
{
  if (L != NULL)
  {
    if (L->nodeType == LIST)
    {
      if (L->data == NULL)
      {
        L->data = CopyDT(elem);
      }
      else
      {
        while (L->next != NULL)
        {
          L = L->next;
        }

        L->next = _CreateSingleDTList();
        L->next->data = CopyDT(elem);
      }
    }
  }
}

Tree Pop(Tree *L)
{
  Tree aux = NULL;
  Tree newTree = NULL;
  Tree prev = NULL;

  if ((*L) == NULL) // Si llega NULL por parametro
  {
    return NULL;
  }

  if ((*L)->nodeType != LIST) // Si no es del tipo correcto
  {
    return NULL;
  }

  if ((*L)->data == NULL) // Si esta vacia
  {
    return NULL;
  }

  if ((*L)->next == NULL) // Si tiene 1 elemento
  {
    newTree = (*L)->data;
    (*L)->data = NULL;
    return newTree;
  }

  aux = *L;

  while (aux->next != NULL)
  {
    prev = aux;
    aux = aux->next;
  }

  newTree = aux->data;
  aux->data = NULL;
  FreeDT(&aux);
  prev->next = NULL;

  return newTree;
}

void FreeDT(Tree *d)
{
  if ((*d) != NULL)
  {
    if ((*d)->nodeType == STR)
    {
      FreeString(&((*d)->dataStr));
      free(*d);
      *d = NULL;

      return;
    }

    if ((*d)->nodeType == DOUBLE)
    {
      free(*d);
      *d = NULL;

      return;
    }

    FreeDT(&(*d)->next);
    FreeDT(&(*d)->data);
    free(*d);
    *d = NULL;
  }
}

int In(Tree set, Tree elem)
{
  if (set == NULL || elem == NULL) // Por si algun parametro ingresado es nulo
  {                                // Debe devolver 0 para la llamada recursiva
    return 0;
  }

  if (set->nodeType != SET) // Por si el tipo de dato no es correcto
  {
    return -1;
  }

  if (set->data == NULL) // Significa que el conjunto esta vacio
  {
    return 0;
  }

  if (CompareDT(set->data, elem))
  {
    return 1;
  }

  return (In(set->next, elem));
}

Tree Union(Tree set1, Tree set2)
{
  Tree newSet = NULL;
  Tree aux = NULL;

  if (set1 == NULL || set2 == NULL)
  {
    return NULL;
  }

  if (set1->nodeType != SET || set2->nodeType != SET) // Asegura el tipo de dato
  {
    return NULL;
  }

  if (set1->data == NULL) // En caso de que el primer conjunto este vacio
  {
    return CopyDT(set2);
  }

  if (set2->data == NULL) // En caso de que el segundo conjunto este vacio
  {
    return CopyDT(set1);
  }

  newSet = CopyDT(set1);
  aux = newSet;

  while (aux->next != NULL) // Se recorre el newSet para agregar
  {                         // los elementos restantes al final
    aux = aux->next;
  }

  while (set2 != NULL) // Se agregan los elementos restantes
  {
    if (!In(set1, set2->data))
    {
      aux->next = _CreateSingleDTSet();
      aux = aux->next;
      aux->data = CopyDT(set2->data);
    }

    set2 = set2->next;
  }

  return newSet;
}

Tree Inter(Tree set1, Tree set2)
{
  Tree newSet = NULL, aux = NULL;

  if (set1 == NULL || set2 == NULL)
  {
    return NULL;
  }

  if (set1->nodeType != SET || set2->nodeType != SET) // Asegura el tipo de dato
  {
    return NULL;
  }

  if (set1->data == NULL || set2->data == NULL) // En caso de que algun conjunto sea vacio,
  {                                             // se devuelve un conjunto vacio.
    return (_CreateSingleDTSet());
  }

  while (set2 != NULL)
  {
    if (In(set1, set2->data))
    {
      if (newSet == NULL)
      {
        newSet = _CreateSingleDTSet();
        newSet->data = CopyDT(set2->data);
        aux = newSet;
      }
      else
      {
        aux->next = _CreateSingleDTSet();
        aux = aux->next;
        aux->data = CopyDT(set2->data);
      }
    }

    set2 = set2->next;
  }

  return newSet != NULL ? newSet : _CreateSingleDTSet();
}

Tree Diff(Tree set1, Tree set2)
{
  Tree newSet = NULL, aux = NULL;

  if (set1 == NULL || set2 == NULL)
  {
    return NULL;
  }

  if (set1->nodeType != SET || set2->nodeType != SET) // Asegura el tipo de dato
  {
    return NULL;
  }

  if (set1->data == NULL) // En caso de que el primer conjunto sea vacio,
  {                       // se devuelve un conjunto vacio.
    return _CreateSingleDTSet();
  }

  if (set2->data == NULL) // En caso de que el segundo conjunto sea vacio,
  {                       // se devuelve una copia del primer conjunto.
    return CopyDT(set1);
  }

  while (set1 != NULL)
  {
    if (!In(set2, set1->data))
    {
      if (newSet == NULL)
      {
        newSet = _CreateSingleDTSet();
        newSet->data = CopyDT(set1->data);
        aux = newSet;
      }
      else
      {
        aux->next = _CreateSingleDTSet();
        aux = aux->next;
        aux->data = set1->data;
      }
    }

    set1 = set1->next;
  }

  return newSet != NULL ? newSet : _CreateSingleDTSet();
}

int IsVoid(Tree set)
{
  if (set == NULL)
  {
    return -1;
  }

  if (set->nodeType != SET)
  {
    return -1;
  }

  if (set->data == NULL)
  {
    return 1;
  }

  if (set->data != NULL)
  {
    return 0;
  }
}

Tree CreateDT2(int type, TString s)
{
  Tree newTree = NULL;

  switch (type)
  {
  case STR:
    return CreateDT(s);

  case SET:
    newTree = _CreateSingleDTSet();
    newTree->data = CreateDT(s);
    return newTree;

  case LIST:
    newTree = _CreateSingleDTList();
    newTree->data = CreateDT(s);
    return newTree;

  default:
    return NULL;
  }
}

Tree CreateDT3(int type, Tree elem)
{
  Tree newTree = NULL;

  if (elem == NULL)
  {
    return NULL;
  }

  switch (type)
  {
  case SET:
    newTree = _CreateSingleDTSet();
    newTree->data = CopyDT(elem);
    return newTree;

  case LIST:
    newTree = _CreateSingleDTList();
    newTree->data = CopyDT(elem);
    return newTree;

  default:
    return NULL;
  }
}

Tree CreateDoubleDT(double value)
{
  Tree newTree = (Tree)malloc(sizeof(struct DataType));

  if (!newTree)
  {
    printf("Error, no hay memoria.");

    return NULL;
  }

  newTree->nodeType = DOUBLE;

  newTree->value = value;

  return newTree;
}

Tree CreateNullDT()
{
  Tree NullDT = NULL;
  return NullDT;
}
