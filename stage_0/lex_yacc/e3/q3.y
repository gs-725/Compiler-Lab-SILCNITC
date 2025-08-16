%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);

char *result;   /* final postfix string */
%}

%union {
    char *string;
}

%token <string> ALPHA
%type <string> expr
%left '+' '-'
%left '*' '/'
%left '(' ')'

%start input

%%

input:
      expr { result = $1; }   /* capture final string */
    ;

expr:
      expr '+' expr   { 
                        $$ = malloc(strlen($1) + strlen($3) + 4);
                        sprintf($$, "+ %s %s", $1, $3);
                        free($1); free($3);
                      }
    | expr '-' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 4);
                        sprintf($$, "- %s %s", $1, $3);
                        free($1); free($3);
                      }
    | expr '*' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 4);
                        sprintf($$, "* %s %s", $1, $3);
                        free($1); free($3);
                      }
    | expr '/' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 4);
                        sprintf($$, "/ %s %s", $1, $3);
                        free($1); free($3);
                      }
    | '(' expr ')'    { $$ = $2; }
    | ALPHA           { $$ = $1; }  /* already strdup'ed in lexer */
    ;

%%

int main() {
    if (yyparse() == 0 && result) {
        printf("%s\n", result);
        free(result);
    }
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

