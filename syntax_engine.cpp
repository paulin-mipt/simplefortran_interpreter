#include <utility>
#include <cstdio>
#include <cstdarg>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "syntax_engine.h"

void yyerror(const char *);

// code_block_t

bool code_block_t::check_declared(const std::string & ID)
{
	auto it = scope_variables.find(ID);
	if (it != scope_variables.end())
	{
		return true;
	}
	else
	{
		if (parent == NULL)
		{
			return false;
		}
		else
		{
			return parent->check_declared(ID);
		}
	}
}

variable_t code_block_t::get_variable(const std::string & ID)
{
	auto it = scope_variables.find(ID);
	if (it == scope_variables.end())
	{
		// trying to search in parent scope
		if (parent == NULL)
		{
			raise_error("undeclared variable: %s", ID.c_str());
		}
		else
		{
			return parent->get_variable(ID);
		}
	}
	else
	{
		return it->second;		
	}
}

void code_block_t::declare_variable(const std::string & ID, variable_type type)
{
	if (check_declared(ID))
	{
		raise_error("redefinition of %s", ID.c_str());
	}

	variable_t var;
	var.is_declared = true;
	var.set_type(type);

	scope_variables.insert(std::make_pair(ID, var));
}

void code_block_t::set_variable(const std::string & ID, variant_t value)
{
	auto it = scope_variables.find(ID);
	if (it == scope_variables.end())
	{
		if (parent != NULL)
		{
			parent->set_variable(ID, value);
		}
		else
		{
			raise_error("assignment to undeclared variable: %s", ID.c_str());
		}
		return;
	}
	
	variable_t & ref = it->second;
	if (!ref.is_assignable_from(value.type))
	{
		raise_error("conversion error: can't convert %s to %s", to_string(value.type), to_string(ref.get_type()));
	}
	ref.is_assigned = true;
	ref.value = value;
}

void code_block_t::declare_set_variable(const std::string & name, variant_t value)
{
	declare_variable(name, value.type);
	set_variable(name, value);
}

// program_t
void program_t::add_method(method_t * method)
{
	auto it = methods.find(method->ID);
	if (it != methods.end())
	{
		raise_error("method redeclaration: %s", method->ID.c_str());
	}
	method->set_block(class_block);
	methods.insert(std::make_pair(method->ID, method));
}

void program_t::set_name(const char * name)
{
	ID = name;
}

void program_t::run()
{
	if (main == NULL)
	{
		raise_error("main method not found");
	}

	fields_declaration->execute(class_block);
	main->run();
}

program_t::program_t()
{
	main = NULL;
	class_block = new code_block_t();
	fields_declaration = new statement_list_t;
}

void program_t::add_field_declaration(statement_t * stmt)
{
	fields_declaration->add(stmt);
}

// method_t

method_t::method_t(const char * name, variable_type return_type, method_signature_t * args)
{
	this->return_type = return_type;
	arguments = args;
	ID = name;
	arguments_passed = 0;
}

void method_t::set_body(statement_list_t * body)
{
	this->body = body;
	body->add(new return_statement_t(this));
}

void method_t::set_block(code_block_t * method_block)
{
	this->method_block = method_block;
}

void method_t::set_return_value(variant_t value)
{
	if (return_type != vtNoType)
	{
		return_value = value;
	}
}

variable_type method_t::get_return_type()
{
	return return_type;
}

const char * method_t::get_id()
{
	return ID.c_str();
}

variant_t method_t::run()
{
	if (arguments_passed != arguments->size())
	{
		raise_error("'%s': too few arguments", ID.c_str());
	}

	arguments_passed = 0;

	method_block->declare_variable(ID, vtInt);

	flow_interruption_type result = body->execute(method_block);

	if (return_type == vtNoType)
	{
		return variant_t();
	}
	else
	{
		return method_block->get_variable(ID).value;
	}
}

void method_t::add_argument(variant_t value)
{
	if (arguments_passed + 1 > arguments->size())
	{
		raise_error("'%s': too many arguments", ID.c_str());
	}

	argument_t * arg = arguments->get_at(arguments_passed);
	method_block->declare_set_variable(arg->ID, value);
	arguments_passed += 1;
}

// while_statement_t

while_statement_t::while_statement_t(expression_t * condition, statement_t * body)
{
	this->condition = condition;
	this->body = body;
}

flow_interruption_type while_statement_t::execute(code_block_t * block)
{
	flow_interruption_type result = fitNoIterruption;

	do
	{
		result = body->execute(block);
		if (result != fitNoIterruption)
		{
			break;
		}
	} while(condition_true(block));

	if (result == fitReturn)
	{
		return fitReturn;
	}
	else
	{
		return fitNoIterruption;
	}
}

bool while_statement_t::condition_true(code_block_t * block)
{
	variant_t cond = condition->eval(block);
	if (cond.type != vtBool)
	{
		raise_error("expected boolean expression in while");
	}
	return cond.bool_value;
}

// statement_list_t
statement_list_t::statement_list_t()
{
}

// return_statement_t
flow_interruption_type return_statement_t::execute(code_block_t * block)
{
	variant_t result = block->get_variable(method->ID).value;
	method->set_return_value(result);
	return fitReturn;
}

// code_block_statement_t

code_block_statement_t::code_block_statement_t(statement_list_t * body)
{
	this->body = body;
}

flow_interruption_type code_block_statement_t::execute(code_block_t * block)
{
	return body->execute(new code_block_t(block));
}

// variable_expression_t 

variant_t variable_expression_t::eval(code_block_t * block)
{
	variable_t var = block->get_variable(ID);
	if (!var.is_assigned)
	{
		raise_error("'%s': using uninitialized variable", ID.c_str());
	}
	return var.value;
}

// conditional_statement_t

conditional_statement_t::conditional_statement_t(expression_t * condition, statement_t * true_way, statement_t * false_way)
{
	this->true_way = true_way;
	this->false_way = false_way;
	this->condition = condition;
}

flow_interruption_type conditional_statement_t::execute(code_block_t * block)
{
	variant_t cond = condition->eval(block);
	if (cond.type != vtBool)
	{
		raise_error("expected boolean expression in if");
	}

	flow_interruption_type result = fitNoIterruption;

	if (cond.bool_value == true)
	{
		result = true_way->execute(block);
	}
	else
	{
		if (false_way != NULL)
		{
			result = false_way->execute(block);
		}
	}
	return result;
}

// binary_expression_t

variant_t binary_expression_t::eval(code_block_t * block)
{
	variant_t value1 = arg1->eval(block);
	variant_t value2 = arg2->eval(block);

	check_types(value1, value2);

	variant_t result;
	switch (type)
	{
	case opAdd:
		result.type = vtInt;
		result.int_value = value1.int_value + value2.int_value;
		break;
	case opSub:
		result.type = vtInt;
		result.int_value = value1.int_value - value2.int_value;
		break;
	case opMul:
		result.type = vtInt;
		result.int_value = value1.int_value * value2.int_value;
		break;
	case opDiv:
		result.type = vtInt;
		result.int_value = value1.int_value / value2.int_value;
		break;
	case opMod:
		result.type = vtInt;
		result.int_value = value1.int_value % value2.int_value;
		break;
	case opAnd:
		result.type = vtBool;
		result.bool_value = value1.bool_value && value2.bool_value;
		break;
	case opOr:
		result.type = vtBool;
		result.bool_value = value1.bool_value || value2.bool_value;
		break;
	case opEquals:
		result.type = vtBool;
		result.bool_value = variant_equals(value1, value2);
		break;
	case opNotEquals:
		result.type = vtBool;
		result.bool_value = !variant_equals(value1, value2);
		break;
	case opLesser:
		result.type = vtBool;
		result.bool_value = value1.int_value < value2.int_value;
		break;
	case opGreater:
		result.type = vtBool;
		result.bool_value = value1.int_value > value2.int_value;
		break;
	case opLesserEquals:
		result.type = vtBool;
		result.bool_value = value1.int_value <= value2.int_value;
		break;
	case opGreaterEquals:
		result.type = vtBool;
		result.bool_value = value1.int_value >= value2.int_value;
		break;
	default:
		break;
	}
	return result;
}

bool binary_expression_t::check_types(variant_t value1, variant_t value2)
{
	switch (type)
	{
	case opAdd:
	case opSub:
	case opMul:
	case opDiv:
	case opMod:
		return value1.type == value2.type && value1.type == vtInt;
	case opAnd:
	case opOr:
		return value1.type == value2.type && value1.type == vtBool;
	case opEquals:
	case opNotEquals:
		return value1.type == value2.type;
		break;
	case opLesser:
	case opGreater:
	case opLesserEquals:
	case opGreaterEquals:
		return value1.type == value2.type && value1.type == vtInt;
	default:
		return false;
	}
}

// invokation_expression_t
variant_t invocation_expression_t::eval(code_block_t * block)
{
	method_t * method = clazz->get_method(method_id);
	if (method == NULL)
	{
		raise_error("'%s': method not found", method_id.c_str());
	}

	method->set_block(new code_block_t());

	for(size_t index = 0; index < params->size(); ++index)
	{
		variant_t value = params->get_at(index)->eval(block);
		method->add_argument(value);
	}
	
	return method->run();
}

// break_statement_t 

flow_interruption_type break_statement_t::execute(code_block_t * block)
{
	return fitBreak;
}

// invoke_statement_t

invoke_statement_t::invoke_statement_t(expression_t * invokee)
{
	this->invokee = invokee;
}

flow_interruption_type invoke_statement_t::execute(code_block_t * block)
{
	invokee->eval(block);
	return fitNoIterruption;
}

// read_statement_t

flow_interruption_type read_statement_t::execute(code_block_t * block)
{
	for(size_t index = 0; index < args->size(); ++index)
	{
		variant_t value;
		value.type = vtInt;
		std::cin >> value.int_value;
		block->set_variable(args->get_at(index), value);
	}
	return fitNoIterruption;
}

// write_arguments_t

std::string write_arguments_t::get_at(size_t index, code_block_t * block)
{
	if (order[index] == 0)
	{
		variant_t result = exprs[exprs_index++]->eval(block);
		return to_string(result);
	}
	else
	{
		return messages[messages_index++];
	}
}

// write_statement_t 

flow_interruption_type write_statement_t::execute(code_block_t * block)
{
	for(size_t index = 0; index < args->size(); ++index)
	{
		std::cout << args->get_at(index, block) << " ";
	}
	std::cout << std::endl;
	return fitNoIterruption;
}

// Utilities

void raise_error(const char * format, ...)
{
	//char message[256];
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	//yyerror(message);
	va_end(args);
	exit(-1);
}

variable_type parse_type(const char * string)
{
	if (!strcmp(string, "int"))
	{
		return vtInt;
	}

	if (!strcmp(string, "boolean"))
	{
		return vtBool;
	}

	return vtNoType;
}


const char * to_string(variable_type type)
{
	switch (type)
	{
	case vtInt:
		return "int";
	case vtBool:
		return "boolean";
	case vtNoType:
		return "undefined type";
	default:
		return NULL;
	}
}

const char * to_string(variant_t value)
{
	char * result = new char[256];
	switch (value.type)
	{
	case vtInt:
		sprintf(result, "%d", value.int_value);
		break;
	case vtBool:
		strcpy(result, value.bool_value ? "true" : "false");
	default:
		break;
	}
	return result;
}

bool variant_equals(variant_t first, variant_t second)
{
	return memcmp(&first, &second, sizeof(variant_t)) == 0;
}
