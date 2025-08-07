%{
#include <stdio.h>
%}

%option noyywrap

%%

\/\/[^\n]*             { printf("Single-line Comment: %s\n", yytext); }

\/\*([^*]|\*+[^*/])*\*+\/   { printf("Multi-line Comment: %s\n", yytext); }

.|\n                  { /* Ignore everything else */ }

%%

int main(int argc, char **argv)
{
    yylex();
    return 0;
}

