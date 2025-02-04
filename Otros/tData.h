#ifndef TDATA_H
#define TDATA_H

#define ATM 1
#define SET 2
#define LIST 3

struct dataType{
	int nodeType;
	union{
		char* atom;
		struct{
			struct dataType* data;
			struct dataType* next; 
		};
	};
};
typedef struct dataType* tData;

/*operaciones generales*/

//a partir de una cadena dada por el usuario, crea un nuevo dato (ATM, SET o LIST)
tData newData(char *s);
//a partir de un tData crea uno nuevo anidando (nest) el tData recibido como hijo derecho
tData newNestedData(tData d, int t);
//elimina un dataSL
void dataFree(tData* d);
//imprime un dato por pantalla
void printData(tData d);
//retorna tipo de dato
int returnType(tData d);
//convierte tData a cadena
char* toStr(tData d);
//copia un tData en otro tData
tData copyData(tData d);

/*operaciones con SET y LIST*/
/*Retorna elemento en posición pos dentro del SLData*/
tData returnElem(tData d, int pos);
int isEmpty(tData d);
int isEqual(tData d1, tData d2);

/*operaciones con LIST*/
//agrega un elemento a la lista por el final 
void PUSH(tData L, tData elem); 
//elimina el último elemento de la lista y lo devuelve como salida
tData POP(tData* L); 
//retorna el tamaño de la lista
int SIZEL(tData L); 

/*operaciones con SET*/
//calcula la cantidad de elementos de un conjunto
int CARDINAL(tData S);
//determina si un elemento pertenece a un conjunto
int IN(tData S, tData elem);
//genera un nuevo conjunto que resulta de la unión de dos conjuntos
tData UNION(tData A,tData B);
//genera un nuevo conjunto que resulta de la intersección de dos conjuntos
tData INTER(tData A,tData B);
//genera un nuevo conjunto que resulta de la diferencia de dos conjuntos
tData DIFF(tData A,tData B);

#endif
