#if !defined(TSTRING_H)
#define TSTRING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char *TString;

void FreeString(TString *s);
int CompareStrings(TString s1, TString s2);
int StringLength(TString s);
TString ReadStr();
TString CreateNullString();

#endif
