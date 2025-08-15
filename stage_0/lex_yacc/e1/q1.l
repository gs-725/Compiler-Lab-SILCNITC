%{
#include "y.tab.h"
%}

%%
[a-zA-Z]    { yylval.character = yytext[0]; return ALPHA; }
[+\-*/]     { return yytext[0]; }
[ \t\n]     ;   /* skip whitespace */
.           { /* ignore unknown characters */ }
%%
int yywrap() { return 1; }

