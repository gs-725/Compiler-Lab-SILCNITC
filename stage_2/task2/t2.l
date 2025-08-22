%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "t2.h"

	int number;
	char variable;
	void yyerror(const char *s);
%}

%%

"begin" {return _BEGIN;}
"end" {return _END;}
"read" {return _READ;}
"write" {return _WRITE;}
[a-z] {variable = *yytext; yylval.node = makevarnode(INTEGER, variable, yytext); return _ID;}
[0-9]+	{number = atoi(yytext); yylval.node = makeconstnode(INTEGER, number, yytext); return _NUM;}
"+" {return _PLUS;}
"-" {return _MINUS;}
"*" {return _MUL;}
"/" {return _DIV;}
"=" {return *yytext;}
[\(\);] { return *yytext; }
[ \t\n]	{}
. {
    yyerror("unknown character\n");
    exit(1);
}

%%

int yywrap() {
	return 1;
}
