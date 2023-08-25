
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tData.h>

// [{q0,q1,q2,q3,q4,q5},{0,1},{[q0,0,{q0}],[q0,1,{q0,q1,q2}],[q1,0,{q2,q3}],[q1,1,{}],[q2,0,{}],[q2,1,{q4}],[q3,0,{q5}],[q3,1,{}],[q4,0,{}],[q4,1,{q5}],[q5,0,{q5}],[q5,1,{q5}]},q0,{q5}]

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
	tData sigmaD = copyData(sigmaND);
	tData transitionsD = newData("{}");
	tData acceptanceStatesD = newData("{}");

	tData aux1;
	tData aux2;

	tData partialState;
	tData newTransition;

	int positionInStateSetD = 1;

	while (positionInStateSetD <= CARDINAL(stateSetD))
	{
		for (int positionInSigmaND = 1; positionInSigmaND <= CARDINAL(sigmaND); positionInSigmaND++)
		{
			partialState = newData("{}");

			for (int positionInStateD = 1; positionInStateD <= CARDINAL(returnElem(stateSetD, positionInStateSetD)); positionInStateD++)
			{
				for (int positionInTransitionsND = 1; positionInTransitionsND <= CARDINAL(transitionsND); positionInTransitionsND++)
				{
					if (isEqual(returnElem(returnElem(stateSetD, positionInStateSetD), positionInStateD), returnElem(returnElem(transitionsND, positionInTransitionsND), 1)))
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

			PUSH(newTransition, copyData(returnElem(stateSetD, positionInStateSetD)));
			PUSH(newTransition, copyData(returnElem(sigmaND, positionInSigmaND)));
			PUSH(newTransition, copyData(partialState));

			aux1 = newNestedData(newTransition, SET);
			dataFree(&newTransition);
			newTransition = NULL;
			aux2 = UNION(transitionsD, aux1);
			dataFree(&transitionsD);
			dataFree(&aux1);
			transitionsD = aux2;
			aux2 = NULL;
			aux1 = NULL;

			if (!IN(stateSetD, partialState))
			{
				aux1 = newNestedData(partialState, SET);
				dataFree(&partialState);
				partialState = aux1;

				aux1 = UNION(stateSetD, partialState);
				dataFree(&stateSetD);
				stateSetD = aux1;
				aux1 = NULL;
			}

			partialState = NULL;
		}

		positionInStateSetD++;
	}

	for (int positionInStateSetD = 1; positionInStateSetD <= CARDINAL(stateSetD); positionInStateSetD++)
	{
		aux1 = INTER(returnElem(stateSetD, positionInStateSetD), acceptanceStatesND);

		if (!isEmpty(aux1))
		{
			dataFree(&aux1);
			aux1 = newNestedData(returnElem(stateSetD, positionInStateSetD), SET);
			aux2 = UNION(acceptanceStatesD, aux1);

			dataFree(&acceptanceStatesD);
			acceptanceStatesD = aux2;
			aux2 = NULL;
		}

		dataFree(&aux1);
	}

	PUSH(AFD, stateSetD);
	PUSH(AFD, sigmaD);
	PUSH(AFD, transitionsD);
	PUSH(AFD, initialStateD);
	PUSH(AFD, acceptanceStatesD);

	return AFD;
}
