%{
#pragma once

#include "bisondef.h"

// forward declarations
int yylex();
void yyerror(const char *);

extern int yylineno;
extern FILE * yyin;

program_t main_program;
variable_type current_type;
method_t * current_method;

%}

%union {
	method_t * method;
	expression_t * expr;
	method_signature_t * args;
	argument_t * arg;
	statement_list_t * stmts;
	statement_t * stmt;
	read_arguments_t * read_args;
	write_arguments_t * write_args;
	parameter_list_t * params;
	variable_type type;
	operation op;
	variant_t value;
	char word[256];
};

// Types
%token <type> TYPE
%token <value> INT
%token <word> STRING

// Comparison operations
%token LE
%token GE
%token NEQ
%token EQ

// Logical operations
%token AND
%token OR

%token <word> ID

// reserved words
%token PROGRAM
%token END
%token FUNCTION
%token SUBROUTINE
%token DOUBLE_DOTS
%token DO
%token IF
%token THEN
%token ELSE
%token WHILE
%token BREAK
%token RETURN
%token CALL
%token READ
%token WRITE

// types

%type <method> main_declaration;
%type <method> main_header;
%type <method> function_declaration;
%type <method> function_header;
%type <method> subroutine_declaration;
%type <method> subroutine_header;
%type <stmts> statement_list;
%type <stmt> statement;
%type <stmt> declaration;
%type <stmts> decl_list;
%type <value> constant;
%type <expr> expression;
%type <expr> term;
%type <stmt> assignment;
%type <stmt> conditional_statement;
%type <stmt> while_statement;
%type <args> decl_params;
%type <args> signature;
%type <params> actual_params;
%type <params> actual_param_list;
%type <expr> invoke_expression;
%type <stmt> io_statement;
%type <read_args> read_param_list;
%type <write_args> write_param_list;
%type <expr> logical_expression;

// some conflict rules
%left '=' NEQ EQ LE GE LT GT
%left '+' '-' OR
%left '*' '/' AND
%nonassoc NOT

%% 
program : /* epsilon */
	| program '\n'
	| program method_declaration

method_declaration : main_declaration
	| function_declaration
	| subroutine_declaration

main_declaration : main_header statement_list END PROGRAM ID
		{
			$$ = $1;
			$$->set_body($2);
			$$->set_block(new code_block_t());
			current_method = NULL;
		}

main_header : PROGRAM ID '\n'
		{
			$$ = new method_t($2, vtNoType, new method_signature_t());
			main_program.set_main($$);
			current_method = $$;
		}

function_header : FUNCTION ID signature '\n'
		{
			$$ = new method_t($2, vtInt, $3);
			current_method = $$;
		}

subroutine_header : SUBROUTINE ID signature '\n'
		{
			$$ = new method_t($2, vtNoType, $3);
			current_method = $$;
		}

function_declaration : function_header statement_list END FUNCTION ID
		{
			$$ = $1;
			$$->set_body($2);
			current_method = NULL;
			main_program.add_method($$);
		} 

subroutine_declaration : subroutine_header statement_list END SUBROUTINE ID
		{
			$$ = $1;
			$$->set_body($2);
			current_method = NULL;
			main_program.add_method($$);
		}

decl_params :'('
		{
			$$ = new method_signature_t();
		}
	|  decl_params ID
		{
			$$ = $1;
			$$->add($2);
		}
	| decl_params ',' ID
		{
			$$ = $1;
			$$->add($3);
		}

signature : decl_params ')'
		{
			$$ = $1;
		}

statement_list : /* epsilon */
		{
			$$ = new statement_list_t();
		}
	| statement_list statement
		{
			$$ = $1;
			$$->add($2);
		}


statement : declaration
		{
			$$ = $1;
		}
	| assignment
		{
			$$ = $1;
		}
	| conditional_statement
		{
			$$ = $1;
		}
	| while_statement
		{
			$$ = $1;
		}
	| io_statement
		{
			$$ = $1;
		}
	| BREAK '\n'
		{
			$$ = new break_statement_t();
		}
	| RETURN '\n'
		{
			$$ = new return_statement_t(current_method);
		}


io_statement : READ read_param_list '\n'
		{
			$$ = new read_statement_t($2);
		}
	| WRITE write_param_list '\n'
		{
			$$ = new write_statement_t($2);
		}	
		
write_param_list : STRING
		{
			$$ = new write_arguments_t();
			$$->add($1);
		}
	| expression
		{
			$$ = new write_arguments_t();
			$$->add($1);
		}
	| write_param_list ',' STRING 
		{
			$$ = $1;
			$$->add($3);
		}
	| write_param_list ',' expression
		{
			$$ = $1;
			$$->add($3);
		}

read_param_list : ID
		{
			$$ = new read_arguments_t();
			$$->add($1);
		}
	| read_param_list ',' ID
		{
			$$ = $1;
			$$->add($3);
		}	

while_statement : DO '\n' statement_list WHILE '(' logical_expression ')' '\n'
		{
			$$ = new while_statement_t($6, $3);
		}

conditional_statement : IF '(' logical_expression ')' THEN '\n' statement_list END IF '\n'
		{
			$$ = new conditional_statement_t($3, $7);
		}
	| IF '(' logical_expression ')' THEN '\n' statement_list ELSE '\n' statement_list END IF '\n'
		{
			$$ = new conditional_statement_t($3, $7, $10);
		}

assignment : ID '=' expression '\n'
		{
			$$ = new assignment_t($1, $3);
		}

declaration : decl_list '\n'
		{
			$$ = $1;
		}

decl_list : TYPE DOUBLE_DOTS ID
		{
			$$ = new statement_list_t();
			current_type = $1;
			$$->add(new declaration_t($3, current_type));
		}
	| decl_list ',' ID
		{
			$$ = $1;
			$$->add(new declaration_t($3, current_type));
		}

expression : expression '+' expression
		{
			$$ = new binary_expression_t(opAdd, $1, $3);
		}
	| expression '-' expression
		{
			$$ = new binary_expression_t(opSub, $1, $3);
		}
	| expression '*' expression
		{
			$$ = new binary_expression_t(opMul, $1, $3);
		}
	| expression '/' expression
		{
			$$ = new binary_expression_t(opDiv, $1, $3);
		}
	| logical_expression AND logical_expression
		{
			$$ = new binary_expression_t(opAnd, $1, $3);
		}
	| logical_expression OR logical_expression
		{
			$$ = new binary_expression_t(opOr, $1, $3);
		}
	| invoke_expression 
		{
			$$ = $1;
		}
	| term
		{
			$$ = $1;
		}

logical_expression : expression LT expression
		{
			$$ = new binary_expression_t(opLesser, $1, $3);
		}
	| expression GT expression
		{
			$$ = new binary_expression_t(opGreater, $1, $3);
		}
	| expression LE expression
		{
			$$ = new binary_expression_t(opLesserEquals, $1, $3);
		}
	| expression GE expression
		{
			$$ = new binary_expression_t(opGreaterEquals, $1, $3);	
		}
	| expression EQ expression
		{
			$$ = new binary_expression_t(opEquals, $1, $3);
		}
	| expression NEQ expression
		{
			$$ = new binary_expression_t(opNotEquals, $1, $3);
		}

invoke_expression : CALL ID actual_param_list
		{
			$$ = new invocation_expression_t($3, $2, &main_program);
		}

actual_params : '(' expression 
		{
			$$ = new parameter_list_t();
			$$->add($2);
		}
	| actual_params ',' expression
		{
			$$ = $1;
			$$->add($3);
		}

actual_param_list : /* epsilon */
		{
			$$ = new parameter_list_t();
		}
	| actual_params ')'
		{
			$$ = $1;
		}

term : constant
		{
			$$ = new constant_t($1);
		}
	| '(' expression ')'
		{
			$$ = $2;
		}
	| ID 
		{
			$$ = new variable_expression_t($1);
		}

constant : INT
		{
			$$ = $1;
		}
%%

void yyerror(const char *s) 
{
	fprintf(stderr, "line number %d: ", yylineno);
	fprintf(stderr, "%s\n", s);
}

int main(int argc, const char* argv[]) 
{
	if (argc < 0) 
	{
		printf("Usage: %s <input_file> [<args>]\n", argv[0]);
		exit(0);
	}
	else 
	{
		yyin = fopen(argv[1], "r");
	} 
	yyparse();
	main_program.run();
	return 0;
}