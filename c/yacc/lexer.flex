/* flex lexer.flex */

%{
#include "y.tab.h"
%}

%option noyywrap

%%
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return TIMES; }
"/"         { return DIVIDE; }
"("         { return LPAREN; }
")"         { return RPAREN; }
[0-9]+      { yylval = atoi(yytext); return INTEGER; }
.           { printf("Illegal character: %s\n", yytext); }
%%
