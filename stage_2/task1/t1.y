%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "t1.h"
	#include "t1.c"

    extern FILE* yyin;

    void yyerror(char const *s);
	int yylex(void);
%}

%union{
	struct tnode *node;
}

%type <node> program stmt_list stmt expr _ID _NUM
%token _PLUS _MINUS _MUL _DIV
%token _BEGIN _END _READ _WRITE _ID _NUM
%left _PLUS _MINUS
%left _MUL _DIV

%%

program : _BEGIN stmt_list _END ';' {
								$$ = $2;
								printf("Parsing Successful\n");

								print_tree($2, 0, 0);
								printf("\n");

								exit(1);
							}
		| _BEGIN _END ';' {
			printf("Empty Program\n");
			printf("Parsing Successful\n");
			exit(1);
		}

stmt_list: stmt_list stmt ';' {$$ = makestnode(STATEMENT, $1, $2, "STATEMENT");}
	| stmt ';' {$$ = $1;}

stmt : _READ '(' _ID ')' { $$ = makestnode(READ, $3, (struct tnode *)NULL, "READ");}
    | _WRITE '(' expr ')' { $$ = makestnode(WRITE, $3, (struct tnode *)NULL, "WRITE");}
    | _ID '=' expr { $$ = makeexprnode(ASSIGNMENT, '=', $1, $3, "="); }
	
expr : expr _PLUS expr		{$$ = makeexprnode(PLUS, '+',$1, $3, "+");}
	| expr _MINUS expr  	{$$ = makeexprnode(MINUS, '-',$1, $3, "-");}
	| expr _MUL expr	{$$ = makeexprnode(MUL, '*',$1, $3, "*");}
	| expr _DIV expr	{$$ = makeexprnode(DIV, '/',$1, $3, "/");}
	| '(' expr ')' 	{$$ = $2;}
	| _NUM		{$$ = $1;}
	| _ID		{$$ = $1;}

%%

void yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(void) 
{
	yyparse();
	
	return 0;
}
