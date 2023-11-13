#include "TString.h"

void FreeString(TString *s)
{
	free(*s);
	*s = NULL;
}

int CompareStrings(TString s1, TString s2)
{
	return (strcmp(s1, s2) == 0);
}

int StringLength(TString s)
{
	return strlen(s);
}

TString ReadStr()
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

TString CreateNullString()
{
	TString s = NULL;
	return s;
}