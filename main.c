
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tData.h>

void showAF(tData AF);
char *leeCad();
tData ConversionAFD(tData AFND);

int main(void)
{
	char *str = leeCad();
	tData AFND = newData(str);
	tData AFD = ConversionAFD(AFND);

	showAF(AFD);

	return 0;
}

void showAF(tData AF)
{
	tData Q, sigma, delta, q0, F;

	Q = returnElem(AF, 1);
	sigma = returnElem(AF, 2);
	delta = returnElem(AF, 3);
	q0 = returnElem(AF, 4);
	F = returnElem(AF, 5);

	printf("\n");
	printf("Q = ");
	printData(Q);
	printf("\n");

	printf("sigma = ");
	printData(sigma);
	printf("\n");

	printf("delta = ");
	printData(delta);
	printf("\n");

	printf("q0 = ");
	printData(q0);
	printf("\n");

	printf("F = ");
	printData(F);
	printf("\n");
}

char *leeCad()
{
	char *cadena = NULL;
	void *aux;
	int t = 0;
	char c;

	cadena = (char *)malloc(sizeof(char));
	if (cadena == NULL)
		return cadena;
	else
	{
		c = getchar();
		if (c != EOF)
		{
			t = 0;
			if (c != '\n')
			{
				cadena[t] = c;
				t++;
			}
			c = getchar();
			while (c != EOF && c != '\n')
			{
				t++;
				aux = (char *)realloc(cadena, sizeof(char) * t);
				if (aux != NULL)
				{
					cadena = aux;
					cadena[t - 1] = c;
				}
				else
					break;
				c = getchar();
			}
			aux = (char *)realloc(cadena, sizeof(char) * (t + 1));
			if (aux != NULL)
			{
				cadena = aux;
				cadena[t] = '\0';
			}
			else
				cadena[t - 1] = '\0';
		}
		else
		{
			cadena[0] = '\0';
		}
		return cadena;
	}
}

tData ConversionAFD(tData AFND)
{
	tData sigmaND = returnElem(AFND, 2);
	tData transitionsND = returnElem(AFND, 3);
	tData initialStateND = returnElem(AFND, 4);
	tData acceptanceStatesND = returnElem(AFND, 5);

	tData AFD = newData("[]");

	tData initialStateD = newNestedData(initialStateND, SET);
	tData stateSetD = newNestedData(initialStateD, SET);
	tData sigmaD = sigmaND;
	tData transitionsD = newData("{}");
	tData acceptanceStatesD = newData("{}");

	tData aux1;
	tData aux2;

	tData partialState;
	tData newTransition;

	int positionInQb = 1;

	while (positionInQb <= CARDINAL(stateSetD))
	{
		for (int positionInSigmaND = 1; positionInSigmaND <= CARDINAL(sigmaND); positionInSigmaND++)
		{
			partialState = newData("{}");

			for (int positionInStateD = 1; positionInStateD <= CARDINAL(returnElem(stateSetD, positionInQb)); positionInStateD++)
			{
				for (int positionInTransitionsND = 1; positionInTransitionsND <= CARDINAL(transitionsND); positionInTransitionsND++)
				{
					if (isEqual(returnElem(returnElem(stateSetD, positionInQb), positionInStateD), returnElem(returnElem(transitionsND, positionInTransitionsND), 1)))
					{
						if (isEqual(returnElem(sigmaND, positionInSigmaND), returnElem(returnElem(transitionsND, positionInTransitionsND), 2)))
						{
							aux1 = UNION(partialState, returnElem(returnElem(transitionsND, positionInTransitionsND), 3));
							dataFree(&partialState);
							partialState = aux1;
							aux1 = NULL;

							break;
						}
					}
				}
			}

			newTransition = newData("[]");

			PUSH(newTransition, returnElem(stateSetD, positionInQb)); // TODO: deberia meter una copia del elemento
			PUSH(newTransition, returnElem(sigmaND, positionInSigmaND));
			PUSH(newTransition, partialState); // Aquí si se guarda el partialState que no es usado en otra parte del AF

			aux1 = newNestedData(newTransition, SET);
			// dataFree(&newTransition);
			newTransition = NULL;
			aux2 = UNION(transitionsD, aux1);
			// dataFree(&transitionsD);
			// dataFree(&aux1);
			transitionsD = aux2;
			aux2 = NULL;
			aux1 = NULL;

			if (!IN(stateSetD, partialState))
			{
				aux1 = newNestedData(partialState, SET);
				// dataFree(&partialState);
				partialState = aux1;

				aux1 = UNION(stateSetD, partialState);
				// dataFree(&stateSetD);
				stateSetD = aux1;
				aux1 = NULL;
			}

			partialState = NULL;
		}

		positionInQb++;
	}

	for (int positionInQb = 1; positionInQb <= CARDINAL(stateSetD); positionInQb++)
	{
		aux1 = INTER(returnElem(stateSetD, positionInQb), acceptanceStatesND);

		// printf("\n");
		// printData(aux1);
		// printf("\n");

		if (!isEmpty(aux1))
		{
			// dataFree(&aux1);
			aux1 = newNestedData(returnElem(stateSetD, positionInQb), SET);
			aux2 = UNION(acceptanceStatesD, aux1);
			// printf("\n");
			// printData(aux1);
			// printf("\n");

			// dataFree(&acceptanceStatesD);
			acceptanceStatesD = aux2;
			aux2 = NULL;
		}

		// dataFree(&aux1);
	}

	PUSH(AFD, stateSetD);
	PUSH(AFD, sigmaD);
	PUSH(AFD, transitionsD);
	PUSH(AFD, initialStateD);
	PUSH(AFD, acceptanceStatesD);

	return AFD;

	// tData acceptanceStatesND = returnElem(AFND, 5);
	// tData stateSetND = returnElem(AFND, 1);
	// tData alphabetND = returnElem(AFND, 2);
	// tData transitionsND = returnElem(AFND, 3);
	// tData initialStateND = returnElem(AFND, 4);
	// tData acceptanceStatesND = returnElem(AFND, 5);

	// tData AFD = newData("[]");
	// tData stateSet = NULL;
	// tData alphabet = NULL;
	// tData transitions = NULL;
	// tData initialState = NULL;
	// tData acceptanceStates = NULL;

	// tData partialState = NULL; // DataType usado para ir haciendo la union de conjuntos de estados
	// tData aux1 = NULL;				 // DataType usado para ir liberando la memoria innecesaria
	// tData aux2 = NULL;				 // DataType usado para ir liberando la memoria innecesaria

	// int position = 1;

	// alphabet = alphabetND; // TODO: CopyDataType
	// initialState = newNestedData(newData(toStr(initialStateND)), SET); // TODO: Dar vuelta
	// stateSet = newNestedData(initialState, SET);
	// transitions = newData("{}");
	// acceptanceStates = newData("{}");

	// while (position <= CARDINAL(stateSet)) // Recorremos todos los elementos del conjunto de estados
	// {																			 // del AFD a medida que se van agregando nuevos.

	// 	for (int i = 1; i <= CARDINAL(alphabet); i++) // Recorremos el alfabeto para definir todas
	// 	{																							// las transiciones con cada caracter.

	// 		partialState = newData("{}");

	// 		for (int j = 1; j <= CARDINAL(returnElem(stateSet, position)); j++) // Recorremos el estado en la posicion n ya que
	// 		{																																		// dicho estado es un conjunto de estados del AFND.

	// 			for (int k = 1; k <= CARDINAL(transitionsND); k++) // Recorremos las transiciones hasta encontrar la que
	// 			{																									 // encaja con el estado y el caracter del alfabeto.

	// 				if (strcmp(toStr(returnElem(returnElem(transitionsND, k), 1)), toStr(returnElem(returnElem(stateSet, position), j))) == 0) // Comparamos el estado
	// 				{
	// 					if (strcmp(toStr(returnElem(returnElem(transitionsND, k), 2)), toStr(returnElem(alphabet, i))) == 0) // Comparamos el caracter
	// 					{
	// 						aux1 = returnElem(returnElem(transitionsND, k), 3);
	// 						aux2 = UNION(aux1, partialState);
	// 						dataFree(&partialState);
	// 						partialState = aux2;
	// 						aux1 = NULL;
	// 						aux2 = NULL;
	// 						break;
	// 					}
	// 				}
	// 			}
	// 		}

	// 		aux1 = newData("[]");
	// 		PUSH(aux1, returnElem(stateSet, position));
	// 		PUSH(aux1, returnElem(alphabet, i));
	// 		PUSH(aux1, partialState);

	// 		aux2 = newNestedData(aux1, SET);
	// 		dataFree(&aux1);
	// 		aux1 = UNION(transitions, aux2); // Agregamos la transicion al conjunto de transiciones
	// 		dataFree(&transitions);
	// 		transitions = aux1;
	// 		aux1 = NULL;

	// 		if (!IN(stateSet, partialState)) // Si el estado no esta (es nuevo), lo agregamos.
	// 		{
	// 			aux1 = newNestedData(partialState, SET);
	// 			dataFree(&partialState);
	// 			partialState = aux1;

	// 			aux1 = UNION(stateSet, partialState);
	// 			dataFree(&stateSet);
	// 			stateSet = aux1;
	// 			aux1 = NULL;
	// 		}

	// 		dataFree(&partialState);
	// 	}

	// 	position++;
	// }

	// for (int i = 1; i <= CARDINAL(stateSet); i++)
	// {
	// 	aux1 = INTER(returnElem(stateSet, i), acceptanceStatesND);

	// 	if (!isEmpty(aux1)) // Si la interseccion entre un estado del AFD y el conjunto de estados de aceptacion del AFND
	// 	{										// es distinta de vacio, se agrega dicho estado al conj. de estados de aceptacion del AFD
	// 		dataFree(&aux1);
	// 		aux1 = newNestedData(returnElem(stateSet, i), SET);
	// 		aux2 = UNION(acceptanceStates, aux1);
	// 		dataFree(&acceptanceStates);
	// 		acceptanceStates = aux2;
	// 		aux2 = NULL;
	// 	}

	// 	dataFree(&aux1);
	// }

	// PUSH(AFD, stateSet);
	// PUSH(AFD, alphabet);
	// PUSH(AFD, transitions);
	// PUSH(AFD, initialState);
	// PUSH(AFD, acceptanceStates);

	// dataFree(&stateSet);
	// dataFree(&alphabet);
	// dataFree(&transitions);
	// dataFree(&initialState);
	// dataFree(&acceptanceStates);

	// return AFD;
}
