%{
    #include<stdio.h>
    #include<stdlib.h>
    #include<string.h>
    #include<ctype.h>

    int yylex(void);
    int yyerror(const char *s);

    char identifier_str[100];
    int id_index;
    int first_char = 1;
%}

%token ALPHABET DIGIT INVALID

%%
identifier:
    ALPHABET rest {
        identifier_str[id_index] = '\0';
        printf("Valid identifier received: %s\n", identifier_str);
        exit(0);
    }
  ;

rest:
      /* empty */
    | ALPHABET rest
    | DIGIT rest
  ;
%%

int yyerror(const char *s) {
    printf("Invalid identifier\n");
    exit(0);
    return 1;
}

int yylex() {
    int c = getchar();

    // End of input
    if (c == EOF) return 0;

    // First char check
    if (first_char) {
        first_char = 0;

        // If first char is newline or space → invalid
        if (c == '\n' || c == ' ' || c == '\t')
            return INVALID;

        if (isalpha(c)) {
            identifier_str[id_index++] = c;
            return ALPHABET;
        }
        return INVALID;
    }

    // For rest of chars
    if (c == '\n' || c == EOF)
        return 0; // End identifier on newline

    if (c == ' ' || c == '\t')
        return INVALID; // Space in middle is invalid

    if (isalpha(c)) {
        identifier_str[id_index++] = c;
        return ALPHABET;
    }
    if (isdigit(c)) {
        identifier_str[id_index++] = c;
        return DIGIT;
    }

    return INVALID;
}

int main() {
    printf("Enter identifier:\n");
    id_index = 0;
    yyparse();
    return 0;
}

