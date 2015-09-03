%option noyywrap

%{
	#pragma once

	#include "bisondef.h"
	
	void yyerror(const char*);
%}

%option yylineno

LETTER ([a-zA-Z])
DIGIT ([0-9])
WORD ({LETTER}|_)({LETTER}|{DIGIT}|_)*
STRING \"[^\"]*\"
NUMBER ([+-]?{DIGIT}+)

%%

{NUMBER} {
		yylval.value.type = vtInt;
		yylval.value.int_value = atoi(yytext);
		return INT;
	}

"INTEGER"	{ 
		yylval.type = vtInt;
		return TYPE; 	
	}

"PROGRAM"	{
		return PROGRAM;
	}

"END"		{
		return END;
	}

"FUNCTION"	{
		return FUNCTION;
	}

"SUBROUTINE" {
		return SUBROUTINE;
	}

"::"		{
		return DOUBLE_DOTS;
	}

"DO"		{
		return DO;
	}

"IF"		{
		return IF;
	}

"THEN"		{
		return THEN;
	}

"ELSE"		{
		return ELSE;
	}

"WHILE"		{
		return WHILE;
	}

"BREAK"		{
		return BREAK;
	}

"RETURN"	{
		return RETURN;
	}

"CALL"		{
		return CALL;
	}

"WRITE"		{
		return WRITE;
	}

"READ"		{
		return READ;
	}
{STRING}	{
		strcpy(yylval.word, yytext);
		return STRING;
	}
	
"AND"	{ return AND;	}
"OR"	{ return OR;	}
"NOT"	{ return NOT;	}
".GE."	{ return GE;	}
".GT."	{ return GT;	}
".LE."	{ return LE;	}
".LT."	{ return LT;	}
"<"		{ return LT;	}
">"		{ return GT;	}
"<="	{ return LE;	}
">="	{ return GE;	}
"=="	{ return EQ;	}
"!="	{ return NEQ;	}
".EQ."	{ return EQ;	}
".NE."	{ return NEQ;	}

{WORD}	{
		strcpy(yylval.word, yytext);
		return ID;
	}


[+-/*\()_{}=;,.\n] { return *yytext;}

[ \t\r] ;

.	{
		yyerror("Unknown symbol\n");
	};
%%