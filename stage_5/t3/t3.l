%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "t3.h"

	int number;
	int yyerror(char const *s);
%}

%option yylineno

alpha [a-zA-Z_]+
digit [0-9]+

%%

"begin" 		{return BEGIN_;}
"end" 			{return END_;}
"read" 			{return READ_;}
"write" 		{return WRITE_;}
"if" 			{return IF_;}
"then" 			{return THEN_;}
"else" 			{return ELSE_;}
"endif" 		{return ENDIF_;}
"while" 		{return WHILE_;}
"do" 			{return DO_;}
"endwhile" 		{return ENDWHILE_;}
"decl" 			{return DECL_;}
"enddecl" 		{return ENDDECL_;}
"int" 			{return INT_;}
"str" 			{return STR_;}
"break" 		{return BREAK_;}
"continue" 		{return CONTINUE_;}
"main" 			{return MAIN_;}
"return" 		{return RETURN_;}

([\"](.)*[\"])						{yylval.ast = makeConstantLeafNode(STRING, 0, yytext); return TEXT_;}
({alpha}({alpha}|{digit})*)         {yylval.ast = makeVariableLeafNode(yytext, yytext); return ID_;}
({digit})	                        {number = atoi(yytext); yylval.ast = makeConstantLeafNode(INTEGER, number, yytext); return NUM_;}

"+" 	{return PLUS_;}
"-" 	{return MINUS_;}
"*" 	{return MUL_;}
"/" 	{return DIV_;}
"%"		{return MOD_;}
"<" 	{return LT_;}
"<=" 	{return LE_;}
">" 	{return GT_;}
">=" 	{return GE_;}
"==" 	{return EQ_;}
"!=" 	{return NE_;}
"&&" 	{return AND_;}
"||" 	{return OR_;}
"=" 	{return *yytext;}
"," 	{return *yytext;}
"{"	    {return *yytext;}
"}"	    {return *yytext;}
[()] 	{return *yytext;}
"["		{return *yytext;}
"]"		{return *yytext;}
";" 	{return *yytext;}
[ \t\n]	{}
. {
    yyerror("unknown character");
    exit(1);
}

%%

int yywrap() {
	return 1;
}
