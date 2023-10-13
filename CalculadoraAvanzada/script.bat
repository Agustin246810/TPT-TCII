@echo off

bison -d fb3-2.y
echo "Bison compilado"
flex fb3-2.l
echo "Flex compilado"
gcc fb3-2.tab.c lex.yy.c fb3-2func.c
echo "gcc compilado"
a.exe
echo "Se ejecuto correctamente"
del a.exe
del fb3-2.tab.c
del fb3-2.tab.h
del lex.yy.c
echo "Se eliminaron los archivos generados"