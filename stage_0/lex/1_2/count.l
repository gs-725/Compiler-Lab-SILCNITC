%{
	#include<stdio.h>
	#include<stdlib.h>
	int integer_count=0;
	int float_count=0;
%}

%option noyywrap
digit [0-9]
integer {digit}+
floating {integer}"."{integer}

%%


{floating}   { float_count++; }
{integer}    { integer_count++; }
[.|\n] {}

%%
int main(int argc, char **argv)  //we need these parameters to take input from cli
{
	yylex();
	printf("Integer Count : %d\n",integer_count);
	printf("Float Count : %d\n",float_count);
	return 1;
	
}
