@echo off

@REM flex ImprimeTyL.l
@REM echo "Flex compilado"
@REM pause
@REM gcc .\lex.yy.c -o Compilado.exe
@REM echo "gcc compilado"
@REM pause
@REM Compilado.exe
@REM echo "Se ejecuto correctamente"
@REM del lex.yy.c
@REM del Compilado.exe
@REM echo "Se eliminaron los archivos generados"
@REM pause

bison -dy Analizador.y
echo "Bison compilado"
@REM pause
flex Analizador.l
echo "Flex compilado"
@REM pause
gcc lex.yy.c y.tab.c
echo "gcc compilado"
@REM pause
a.exe
echo "Se ejecuto correctamente"
@REM pause
del a.exe
del lex.yy.c
del y.tab.c
del y.tab.h
echo "Se eliminaron los archivos generados"