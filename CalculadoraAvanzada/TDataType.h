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

typedef struct DataType *tData;

/*OPERACIONES GENERALES*/

// A partir de una cadena dada por el usuario, crea un nuevo dato (STR, SET o LIST)
// considerar la posibilidad que un conjunto o lista pueden ser vacias
tData CreateDT(TString s);
// Crea un nuevo DataType a partir de una cadena y un tipo (STR, SET o LIST)
tData CreateDT2(int type, TString s);
// Crea un nuevo DataType de tipo SET o LIST con un unico elemento (una copia del elemento dado)
tData CreateDT3(int type, tData elem);
// Crea un DataType de tipo double con un valor determinado
tData CreateDoubleDT(double value);
// Devuelve un DataType nulo.
tData CreateNullDT();
// Retorna un puntero a una copia de un DataType dado.
tData CopyDT(tData original);
// Elimina un dato
void FreeDT(tData *d);
// Imprime un dato por pantalla
void PrintDT(tData tree);
// Retorna tipo de dato
int TypeDT(tData d);
// Retorna 0 cuando los elementos son distintos y 1 cuando son iguales.
int CompareDT(tData elem1, tData elem2);

/*Operaciones con STR*/

// Retorna cadena en hijo izquierdo, si hijo izq no es cadena retorna NULL
TString StrDT(tData d);

/*Operaciones con DOUBLE*/

// Retorna el valor guardado en un DataType de tipo DOUBLE, en caso de que
// no coincida el tipo, devuelve 0 y muestra un mensaje de error.
double ValueDT(tData d);

/*Operaciones con SET y LIST*/

// Devuelve el elemento en una determinada posicion
// dentro de un conjunto o una lista. (No una copia)
tData ElemDT(tData CL, int pos);

/*Operaciones con LIST*/

// Agrega una copia de un elemento a la lista por el final
void Push(tData L, tData elem);
// Elimina el ultimo elemento de la lista y lo devuelve como salida
tData Pop(tData *L);
// Retorna el tamanio de la lista
int SizeL(tData L);

/*Operaciones con SET*/
// Todas las operciones retornan -1 o NULL cuando no corresponde el tipo

// Calcula la cantidad de elementos de un conjunto
int Cardinal(tData S);
// Determina si un elemento pertenece a un conjunto (1: pertenece, 0: no pertenece)
int In(tData set, tData elem);
// Genera un nuevo conjunto que resulta de la union de dos conjuntos
tData Union(tData set1, tData set2);
// Genera un nuevo conjunto que resulta de la interseccion de dos conjuntos
tData Inter(tData set1, tData set2);
// Genera un nuevo conjunto que resulta de la diferencia de dos conjuntos (set1 - set2)
tData Diff(tData set1, tData set2);
// Devuelve 1 si el conjunto es vacio y 0 si no lo es.
// Devuelve -1 en caso de que haya algun error en el parametro set.
int IsVoid(tData set);
// Retorna 1 cuando el primer conjunto esta
// contenido en el otro 0 cuando no lo esta.
int IsContained(tData set1, tData set2);

#endif