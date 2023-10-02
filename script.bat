@echo off

flex ImprimeTyL.l
echo "Flex compilado"
pause
gcc .\lex.yy.c -o Compilado.exe
echo "gcc compilado"
pause
Compilado.exe
echo "Se ejecuto correctamente"
del lex.yy.c
del Compilado.exe
echo "Se eliminaron los archivos generados"
pause

@REM bison -dy Analizador.y
@REM echo "Bison compilado"
@REM flex Analizador.l
@REM echo "Flex compilado"
@REM gcc lex.yy.c y.tab.c
@REM echo "gcc compilado"
@REM a.exe
@REM echo "Se ejecuto correctamente"
@REM del a.exe
@REM del lex.yy.c
@REM del y.tab.c
@REM del y.tab.h
@REM echo "Se eliminaron los archivos generados"