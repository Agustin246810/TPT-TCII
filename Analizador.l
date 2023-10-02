%{
void yyerror(char *s);

/* TOKENS */
enum yytokentype {
  IF = 258,
  ELSE = 259,
  WHILE = 260,
  FOREACH = 261,
  ELEM = 262,
  SET = 263,
  LIST = 264,
  POP = 265,
  PUSH = 266,
  TO = 267,
  ID = 268,
  IN = 269,
  UNION = 270,
  INTERS = 271,
  DIFF = 272,
  NOT = 273,
  AND = 274,
  OR = 275,
  ISEQUAL = 276,
  NOTEQUAL = 277
};

%}

%option noyywrap

%%

"if"                        { return IF; }
"else"                      { return ELSE; }
"while"                     { return WHILE; }
"foreach"                   { return FOREACH; }
"set"                       { return SET; }
"list"                      { return LIST; }
"pop"                       { return POP; }
"push"                      { return PUSH; }
"to"                        { return TO; }
"in"                        { return IN; }
"=="                        { return ISEQUAL; }
"!="                        { return NOTEQUAL; }
"&&"                        { return AND; }
"||"                        { return OR; }
"!"                         { return NOT; }

"|"                         { return UNION; }
"&"                         { return INTERS; }
"&!"                        { return DIFF; }

"[" |
"]" |
"{" |
"}" |
"(" |
")" |
"," |
";" |
"="                         { return yytext[0]; }

"'"[a-zA-Z0-9]+             { return ELEM; }
[a-zA-Z][a-zA-Z0-9]*        { return ID; }

"\n"                        { /* Ignora saltos de linea */ }
"//".*                      { /* Ignora comentarios */ }
[ \t]                       { /* Ignora espacios en blanco */ }
.                           yyerror(yytext);

%%
