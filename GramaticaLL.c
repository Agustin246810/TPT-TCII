
void match(char cadena[100])
{
  if (strcmp(lexeme, cadena) == 0)
    lex();
  else
  {
    printf("ERROR-Se esperaba %s y se encontro %s\n", cadena, lexeme);
    error = 1;
    exit(0);
  }
}

void exp()
{
  printf("<exp>\n");
  complexop();
  exprec();
}

void exprec()
{
  if (nextToken == CMP)
  {
    match(lexeme);
    complexop();
    exprec();
  }
}

void logicop()
{
  if (nextToken == NOT)
  {
    match("!");
    exp();
    logicoprec();
  }
  else
  {
    exp();
    logicoprec();
  }
}

void logicoprec()
{
  if (nextToken == LOGICOP)
  {
    match(lexeme);
    exp();
    logicoprec();
  }
}

void complexop()
{
  if (nextToken == POP)
  {
    match("pop");
    literals();
    complexoprec();
  }
  else
  {
    literals();
    position();
    complexoprec();
  }
}

void complexoprec()
{
  if (nextToken == SETOP)
  {
    match(lexeme);
    literals();
    complexoprec();
  }
}

void position()
{
  if (nextToken == '[')
  {
    match("[");
    literals();
    match("]");
  }
}

void literals()
{
  if (nextToken == NAME || nextToken == ELEM || nextToken == '(')
  {
    if (nextToken == NAME || nextToken == ELEM)
    {
      match(lexeme);
    }
    else
    {
      match("(");
      exp();
      match(")");
    }
  }
  else
  {
    if (nextToken == '{')
    {
      litealset();
    }
    else
    {
      literalist();
    }
  }
}

void literalset()
{
  match("{");
  explist2();
  match("}");
}

void literalist()
{
  match("[");
  explist2();
  match("]");
}

void explist2()
{
  if (nextToken == POP || nextToken == NAME || nextToken == ELEM || nextToken == '(' || nextToken == '{' || nextToken == '[')
  {
    explist();
  }
}

void explist()
{
  exp();
  explistrec();
}

void explistrec()
{
  if (nextToken == ',')
  {
    match(",");
    exp();
    explistrec();
  }
}
