/* 
    yacc -d calc.y
    yacc -dv calc.y (to verify grammar)
    gcc y.tab.c -o calc
*/

%{
#include <stdio.h>
#include <stdlib.h>
#include "lex.yy.c"
void yyerror(char const *s);
%}

%token INTEGER
%token PLUS '+'
%token MINUS '-'
%token TIMES '*'
%token DIVIDE '/'
%token LPAREN '('
%token RPAREN ')'

%%
expression : expression '+' term    { $$ = $1 + $3; }
           | expression '-' term    { $$ = $1 - $3; }
           | term                   { $$ = $1; }
term       : term '*' factor        { $$ = $1 * $3; }
           | term '/' factor        { $$ = $1 / $3; }
           | factor                 { $$ = $1; }
factor     : '(' expression ')'     { $$ = $2; }
           | INTEGER                { $$ = $1; }
%%

int main(void) {
    yylex();
    yyparse();
}

void yyerror(char const *s) {
    fprintf(stderr, "error: %s\n", s);
}


