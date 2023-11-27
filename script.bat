@echo off

bison -d fb3-2.y
flex fb3-2.l
gcc fb3-2.tab.c lex.yy.c fb3-2func.c TDataType.c TString.c
a.exe
echo "Se ejecuto correctamente"
del a.exe
del fb3-2.tab.c
del fb3-2.tab.h
del lex.yy.c
echo "Se eliminaron los archivos generados"