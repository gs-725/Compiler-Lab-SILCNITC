%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);

int lvl = 0;
int max_lvl = 0;
%}

/* Token definitions */
%token IF ID NUMBER RELOP SEMICOLON LBRACE RBRACE LPAREN RPAREN

/* Operator precedence and associativity */
%left RELOP

%%

start:
    statement_list { printf("\nComplete: max nesting level = %d\n", max_lvl); }
    ;

statement_list:
      /* empty */
    | statement_list statement
    ;

statement:
      simple_stmt
    | if_stmt
    ;

simple_stmt:
      ID SEMICOLON
    ;

if_stmt:
      IF LPAREN condition RPAREN LBRACE { 
          lvl++;
          if (max_lvl < lvl) max_lvl = lvl;
      } statement_list RBRACE
      {
          lvl--;  /* end of this IF */
      }
    ;

condition:
      expression RELOP expression
    ;

expression:
      ID
    | NUMBER
    | LPAREN expression RPAREN
    ;

%%

void yyerror(const char *s)
{
    printf("Error: %s\n", s);
}

int yylex(void)
{
    int c;

    /* skip spaces, tabs, newlines */
    while ((c = getchar()) == ' ' || c == '\t' || c == '\n') {}

    if (c == EOF) return 0;

    /* identifiers (IF or variable) */
    if (isalpha(c)) {
        char word[32];
        int i = 0;
        word[i++] = c;
        while ((c = getchar()) != EOF && isalnum(c)) {
            word[i++] = c;
        }
        word[i] = '\0';

        if (c != EOF) ungetc(c, stdin);

        if (strcmp(word, "if") == 0)
            return IF;

        return ID;
    }

    /* numbers */
    if (isdigit(c)) {
        while ((c = getchar()) != EOF && isdigit(c)) {}
        if (c != EOF) ungetc(c, stdin);
        return NUMBER;
    }

    /* relational operators */
    if (c == '<' || c == '>' || c == '=' || c == '!') {
        int next = getchar();
        if (next == '=') return RELOP;
        ungetc(next, stdin);
        return RELOP;
    }

    /* single-character tokens */
    if (c == ';') return SEMICOLON;
    if (c == '{') return LBRACE;
    if (c == '}') return RBRACE;
    if (c == '(') return LPAREN;
    if (c == ')') return RPAREN;

    return c;
}

int main(void)
{
    printf("Enter your code:\n");
    yyparse();
    return 0;
}

