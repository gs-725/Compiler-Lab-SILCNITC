%{
	#include<stdio.h>
	#include<stdlib.h>
%}

%option noyywrap

upp     [A-Z]
reqtxt  A{upp}{upp}

%%

{reqtxt}   { printf("%s\n", yytext); }

.|\n       { /* ignore all other characters and newlines */ }

%%

int main(int argc, char **argv)
{
    yylex();
    return 0;
}

