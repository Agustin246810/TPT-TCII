#ifndef DATATYPE_H
#define DATATYPE_H

#define STR 1
#define SET 2
#define LIST 3
#define DOUBLE 4

#include "TString.h"

struct DataType
{
	int nodeType;
	union
	{
		char *dataStr;
		double value;
		struct
		{
			struct DataType *data;
			struct DataType *next;
		};
	};
};

typedef struct DataType *Tree;

/*OPERACIONES GENERALES*/

// A partir de una cadena dada por el usuario, crea un nuevo dato (STR, SET o LIST)
// considerar la posibilidad que un conjunto o lista pueden ser vacias
Tree CreateDT(TString s);
// Crea un nuevo DataType a partir de una cadena y un tipo (STR, SET o LIST)
Tree CreateDT2(int type, TString s);
// Crea un nuevo DataType de tipo SET o LIST con un unico elemento (una copia del elemento dado)
Tree CreateDT3(int type, Tree elem);
// Crea un DataType de tipo double con un valor determinado
Tree CreateDoubleDT(double value);
// Devuelve un DataType nulo.
Tree CreateNullDT();
// Retorna un puntero a una copia de un DataType dado.
Tree CopyDT(Tree original);
// Elimina un dato
void FreeDT(Tree *d);
// Imprime un dato por pantalla
void PrintDT(Tree tree);
// Retorna tipo de dato
int TypeDT(Tree d);

/*Operaciones con STR*/

// Retorna cadena en hijo izquierdo, si hijo izq no es cadena retorna NULL
TString StrDT(Tree d);

/*Operaciones con DOUBLE*/

// Retorna el valor guardado en un DataType de tipo DOUBLE, en caso de que
// no coincida el tipo, devuelve 0 y muestra un mensaje de error.
double ValueDT(Tree d);

/*Operaciones con SET y LIST*/

// Devuelve el elemento en una determinada posicion
// dentro de un conjunto o una lista.
Tree ElemDT(Tree CL, int pos);

/*Operaciones con LIST*/

// Agrega una copia de un elemento a la lista por el final
void Push(Tree L, Tree elem);
// Elimina el ultimo elemento de la lista y lo devuelve como salida
Tree Pop(Tree *L);
// Retorna el tamanio de la lista
int SizeL(Tree L);

/*Operaciones con SET*/
// Todas las operciones retornan -1 o NULL cuando no corresponde el tipo

// Calcula la cantidad de elementos de un conjunto
int Cardinal(Tree S);
// Determina si un elemento pertenece a un conjunto (1: pertenece, 0: no pertenece)
int In(Tree set, Tree elem);
// Genera un nuevo conjunto que resulta de la union de dos conjuntos
Tree Union(Tree set1, Tree set2);
// Genera un nuevo conjunto que resulta de la interseccion de dos conjuntos
Tree Inter(Tree set1, Tree set2);
// Genera un nuevo conjunto que resulta de la diferencia de dos conjuntos (set1 - set2)
Tree Diff(Tree set1, Tree set2);
// Devuelve 1 si el conjunto es vacio y 0 si no lo es.
// Devuelve -1 en caso de que haya algun error en el parametro set.
int IsVoid(Tree set);

#endif