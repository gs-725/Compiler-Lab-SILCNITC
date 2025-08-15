%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);

char *result;
%}

%union {
    char character;
    char *string;
}

%token <character> ALPHA
%type <string> expr
%left '+' '-'
%left '*' '/'
%left '(' ')'

%%
expr:
      expr '+' expr   { 
                        $$ = malloc(strlen($1) + strlen($3) + 2);
                        sprintf($$, "%s%s+", $1, $3);
                        free($1); free($3);
                        result = $$;
                      }
    | expr '-' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 2);
                        sprintf($$, "%s%s-", $1, $3);
                        free($1); free($3);
                        result = $$;
                      }
    | expr '*' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 2);
                        sprintf($$, "%s%s*", $1, $3);
                        free($1); free($3);
                        result = $$;
                      }
    | expr '/' expr   {
                        $$ = malloc(strlen($1) + strlen($3) + 2);
                        sprintf($$, "%s%s/", $1, $3);
                        free($1); free($3);
                        result = $$;
                      }
    | '(' expr ')'    { $$ = strdup($2); free($2); }
    | ALPHA           { $$ = strdup((char[]){$1, '\0'}); }
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

