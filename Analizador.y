%{
  #include <stdio.h>

  int yylex(void);
  void yyerror(char *s);
%}

/* Se definen los tokens */
%token IF ELSE WHILE FOREACH ELEM SET LIST POP PUSH TO ID IN

/* Especificamos la variable inicial */
%start init

%left ','
%left ';'

%left UNION
%left INTERS
%left DIFF

%left NOT
%left AND
%left OR
%left ISEQUAL
%left NOTEQUAL

%%
  init
    : if ';'
    {
      printf("\nSentencia IF escrita correctamente");
    }

    | while ';'
    {
      printf("\nSentencia WHILE escrita correctamente");
    }

    | foreach ';'
    {
      printf("\nSentencia FOREACH escrita correctamente");
    }

    | definition ';'
    {
      printf("\nSentencia DEFINICION escrita correctamente");
    }

    | assignment ';'
    {
      printf("\nSentencia ASIGNACION escrita correctamente");
    }

    | multiAssignment ';'
    {
      printf("\nSentencia ASIGNACION MULTIPLE escrita correctamente");
    }

    | push ';'
    {
      printf("\nSentencia PUSH escrita correctamente");
    }

    | pop ';'
    {
      printf("\nSentencia POP escrita correctamente");
    }
  ;

  /* no admite definicion de tipo elem */
  definition
    : SET ID
    | LIST ID
  ;

  if
    : IF '(' logicExpression ')' '{' statementList '}'
    | IF '(' logicExpression ')' '{' statementList '}' ELSE '{' statementList '}'
  ;

  statementList
    : statement ';' statementList
    | statement ';'
  ;

  statement
    : if
    | while
    | foreach
    | definition
    | assignment
    | multiAssignment
    | pop
    | push
  ;

  logicExpression
    : '(' logicExpression ')'
    | and
    | or
    | not
    | isEqual
    | notEqual
  ;

  and
    : logicExpression AND logicExpression
  ;

  or
    : logicExpression OR logicExpression
  ;

  not
    : NOT logicExpression
  ;

  isEqual
    : expression ISEQUAL expression
  ;

  notEqual
    : expression NOTEQUAL expression
  ;

  while
    : WHILE '(' logicExpression ')' '{' statementList '}'
    | WHILE '(' logicExpression ')' '{' '}'
  ;

  foreach
    : FOREACH ID IN expression '{' statementList '}'
    | FOREACH ID IN expression '{' '}'
  ;

  assignment
    : ID '=' expression
  ;

  multiAssignment
    : '(' idList ')' '=' '(' expressionList ')'
  ;

  idList
    : ID
    | ID ',' idList
  ;

  expression
    : '(' expression ')'
    | set
    | list
    | ELEM
    | ID
    | pop
  ;

  /* No admite lista de expresiones vacia */
  expressionList
    : expression ',' expressionList
    | expression
  ;
  
  set
    : '{' '}'
    | '{' expressionList '}'
    | union
    | inters
    | diff
  ;

  union
    : set UNION set
  ;

  inters
    : set INTERS set
  ;

  diff
    : set DIFF set
  ;

  list
    : '[' ']'
    | '[' expressionList ']'
  ;

  pop
    : POP list
  ;

  push
    : PUSH expression TO list
  ;

%%

int main(void)
{
  return (yyparse());
}

void yyerror(char *s)
{
  printf("\n%s", s);
}

int yywrap()
{
  return 1;
}