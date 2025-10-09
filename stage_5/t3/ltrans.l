%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "ltrans.c"
    int first = 1, addr = 2040;

    FILE *fp;
%}

%%

(L|F)[0-9]+:\n  {
        if (first) {
            char *label_name = malloc(strlen(yytext));
            strcpy(label_name, yytext);
            label_name[strlen(yytext) - 2] = '\0';
            append(label_name, addr);
        }
    }

(L|F)[0-9]+ {
    if (!first) {
        fprintf(fp, "%d", getAddr(yytext));
    }
}


\n {
    addr += 2;
    if (!first) {
            fprintf(fp, "\n");
        }
    }

. {
    if (!first) {
        fprintf(fp, "%s", yytext);
    }
}

%%

int yywrap() {
    if (first) {
        yyin = fopen("output.xsm", "r");
        first = 0;
        return 0;
    }
    if (!first) {
        return 1;
    }
}

int main() {
    fp = fopen("code.xsm", "w");
    extern FILE *yyin;
    yyin = fopen("output.xsm", "r");
    yylex();
    printf("Code generated Successfully\n");
    return 0;
}
