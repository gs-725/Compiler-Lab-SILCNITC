%{
#include <stdio.h>

int line_count = 0;
int word_count = 0;
int char_count = 0;
%}

%option noyywrap

LETTER     [a-zA-Z]
DIGIT      [0-9]
ID         {LETTER}({LETTER}|{DIGIT})*

%%

\n                  { line_count++; char_count++; }              // Newline
[ \t]+              { char_count += yyleng; }                    // Whitespace
{ID}                { word_count++; char_count += yyleng; }      // C-style identifiers
.              { char_count+=yyleng; }                            // Any other character

%%

int main(int argc, char **argv) {
    yylex();
    printf("Lines: %d\n", line_count);
    printf("Words: %d\n", word_count);
    printf("Characters: %d\n", char_count);
    return 0;
}

