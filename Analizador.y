%{
  #include <stdio.h>

  int yylex(void);
  void yyerror(char *s);
%}

/* Se definen los tokens */
%token IF ELSE WHILE FOREACH ELEM SET LIST POP PUSH TO IN ID AND OR NOT ISEQUAL NOTEQUAL UNION INTERS DIFF

/* Especificamos la variable inicial */
%start init

%left AND OR
%left ISEQUAL NOTEQUAL
%left UNION INTERS DIFF

%%
  init
    : if
    {
      printf("\nSentencia IF escrita correctamente");
    }

    | while
    {
      printf("\nSentencia WHILE escrita correctamente");
    }

    | foreach
    {
      printf("\nSentencia FOREACH escrita correctamente");
    }

    | definition
    {
      printf("\nSentencia DEFINICION escrita correctamente");
    }

    | assignment
    {
      printf("\nSentencia ASIGNACION escrita correctamente");
    }

    | multiAssignment
    {
      printf("\nSentencia ASIGNACION MULTIPLE escrita correctamente");
    }

    | push
    {
      printf("\nSentencia PUSH escrita correctamente");
    }

    | pop
    {
      printf("\nSentencia POP escrita correctamente");
    }
  ;

  definition
    : SET ID
    | LIST ID
    | ELEM ID
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
  ;

  foreach
    : FOREACH ID IN set '{' statementList '}'
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
    : expression
    | expression ',' expressionList
  ;
  
  set
    : '{' '}'
    | '{' expressionList '}'
    | union
    | inters
    | diff
    | ID
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
    | ID
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