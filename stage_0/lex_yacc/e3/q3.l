%{
#include "y.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%%
[a-zA-Z]+    { yylval.string = strdup(yytext); return ALPHA; }
[+\-*/]      { return yytext[0]; }
[ \t\n]      ;   /* skip whitespace */
.            { /* ignore unknown characters */ }
%%

int yywrap() { return 1; }

