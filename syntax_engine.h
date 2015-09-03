#pragma once

#include <map>
#include <vector>
#include <string>

#include "types.h"

class method_t;
class method_signature_t;
class code_block_t;
class expression_t;
class statement_list_t;
class parameter_list_t;

class expression_t 
{
public:
	virtual variant_t eval(code_block_t * block) = 0;
};

class statement_t
{
public:
	virtual flow_interruption_type execute(code_block_t * block) = 0;
};

class code_block_t
{
private:
	std::map<std::string, variable_t> scope_variables;
	std::vector<code_block_t *> blocks;

	bool check_declared(const std::string & name);

public:
	code_block_t * parent;

	code_block_t(code_block_t * parent = NULL)
	{
		this->parent = parent;
	}

	void add_sub_block(code_block_t * sub_block)
	{
		blocks.push_back(sub_block);
	}

	void clear_scope()
	{
		scope_variables.clear();
	}

	variable_t get_variable(const std::string & name);
	void declare_variable(const std::string & name, variable_type type);
	void set_variable(const std::string & name, variant_t value);
	void declare_set_variable(const std::string & name, variant_t value);
	
};

class program_t
{
protected:
	code_block_t * class_block;
	statement_list_t * fields_declaration;
	method_t * main;
	std::map<std::string, method_t *> methods;
	std::string ID;
public:
	program_t();

	void set_name(const char * name);

	code_block_t * get_code_block() 
	{
		return class_block;
	}

	void set_main(method_t * main)
	{
		this->main = main;
	}

	method_t * get_method(const std::string & ID)
	{
		auto it = methods.find(ID);
		if (it == methods.end())
		{
			return NULL;
		}
		
		return it->second;
	}

	void add_field_declaration(statement_t * stmt);

	void run();
	void add_method(method_t * method);
};

class method_t 
{
protected:
	code_block_t * method_block;
	variable_type return_type;
	method_signature_t * arguments;
	statement_list_t * body;
	variant_t return_value;
	size_t arguments_passed;

public:
	std::string ID;

	method_t(const char * name, variable_type return_type, method_signature_t * args);
	void set_body(statement_list_t * body);
	void set_block(code_block_t * method_block);
	void set_return_value(variant_t value);
	variable_type get_return_type();
	virtual void add_argument(variant_t value);
	virtual variant_t run();
	const char * get_id();
};

class argument_t 
{
private:
	std::string ID;

public:
	argument_t(const std::string & name)
	{
		this->ID = name;
	}

	friend class method_t;
};

class method_signature_t
{
private:
	std::vector<argument_t *> args;
public:
	size_t size()
	{
		return args.size();
	}

	argument_t * get_at(size_t index)
	{
		return args[index];
	}

	void add(const char * name)
	{
		args.push_back(new argument_t(name));
	}
};

class statement_list_t : public statement_t
{
private:
	std::vector<statement_t *> statements;
public:
	statement_list_t();

	void add(statement_t * stmt)
	{
		statements.push_back(stmt);
	}

	flow_interruption_type execute(code_block_t * block)
	{
		flow_interruption_type result;
		for(auto it = statements.begin(); it != statements.end(); ++it)
		{
			result = (*it)->execute(block);
			if (result != fitNoIterruption)
			{
				return result;
			}
		}
		return fitNoIterruption;
	}
};

class code_block_statement_t : public statement_list_t
{
private:
	statement_list_t * body;
public:
	code_block_statement_t(statement_list_t * body);
	flow_interruption_type execute(code_block_t * block);
};

class declaration_t : public statement_t
{
private:
	std::string ID;
	variable_type type;
public:
	declaration_t(const char * name, variable_type type)
	{
		this->ID = name;
		this->type = type;
	}

	flow_interruption_type execute(code_block_t * block)
	{
		block->declare_variable(ID, type); 
		return fitNoIterruption;
	}
};

class assignment_t : public statement_t
{
private:
	std::string ID;
	expression_t * value;
public:
	assignment_t(const char * name, expression_t * value)
	{
		this->ID = name;
		this->value = value;
	}

	flow_interruption_type execute(code_block_t * block)
	{
		block->set_variable(ID, value->eval(block));
		return fitNoIterruption;
	}
};

class read_arguments_t 
{
private:
	std::vector<std::string> params;
public:
	size_t size()
	{
		return params.size();
	}

	std::string get_at(size_t index)
	{
		return params[index];
	}

	void add(const char * name)
	{
		params.push_back(name);
	}
};

class read_statement_t : public statement_t 
{
private:
	read_arguments_t * args;
public:
	read_statement_t(read_arguments_t * args)
	{
		this->args = args;
	}

	flow_interruption_type execute(code_block_t * block);
};

class write_arguments_t 
{
private:
	std::vector<expression_t *> exprs;
	std::vector<std::string> messages;
	std::vector<int> order;

	size_t exprs_index;
	size_t messages_index;

public:
	write_arguments_t()
	{
		exprs_index = 0;
		messages_index = 0;
	}

	void add(expression_t * expr)
	{
		exprs.push_back(expr);
		order.push_back(0);
	}

	void add(std::string message)
	{
		messages.push_back(message.substr(1, message.size()-2));
		order.push_back(1);
	}

	size_t size()
	{
		return order.size();
	}

	std::string get_at(size_t index, code_block_t * block);
};

class write_statement_t : public statement_t
{
private:
	write_arguments_t * args;
public:
	write_statement_t(write_arguments_t * args)
	{
		this->args = args;
	}

	flow_interruption_type execute(code_block_t * block);
};

class return_statement_t : public statement_t
{
private:
	method_t * method;
public:
	return_statement_t(method_t * method)
	{
		this->method = method;
	}

	flow_interruption_type execute(code_block_t * block);
};

class constant_t : public expression_t
{
private:
	variant_t value;
public:
	constant_t(variant_t value)
	{
		this->value = value;
	}

	virtual variant_t eval(code_block_t * block)
	{
		return value;
	}
};

class variable_expression_t : public expression_t 
{
private:
	std::string ID;
public:
	variable_expression_t(const char * name)
	{
		this->ID = name;
	}

	variant_t eval(code_block_t * block);
};

class binary_expression_t : public expression_t 
{
private:
	expression_t * arg1;
	expression_t * arg2;
	operation type;

	bool check_types(variant_t value1, variant_t value2);

public:
	binary_expression_t(operation type, expression_t * arg1, expression_t * arg2)
	{
		this->type = type;
		this->arg1 = arg1;
		this->arg2 = arg2;
	}

	variant_t eval(code_block_t * block);
};

class invocation_expression_t : public expression_t
{
private:
	parameter_list_t * params;
	std::string method_id;
	program_t * clazz;
public:
	invocation_expression_t(parameter_list_t * params, const char * method_name, program_t * clazz)
	{
		this->params = params;
		method_id = method_name;
		this->clazz = clazz;
	}

	variant_t eval(code_block_t * block);
};

class parameter_list_t 
{
private:
	std::vector<expression_t *> params;
public:
	parameter_list_t()
	{
	}

	size_t size()
	{
		return params.size();
	}

	expression_t * get_at(size_t index)
	{
		if (index >= params.size())
		{
			return NULL;
		}
		return params[index];
	}

	void add(expression_t * expr)
	{
		params.push_back(expr);
	}
};

class conditional_statement_t : public statement_t
{
private:
	statement_t * true_way;
	statement_t * false_way;
	expression_t * condition;
public:
	conditional_statement_t(expression_t * condition, statement_t * true_way, statement_t * false_way = NULL);
	flow_interruption_type execute(code_block_t * block);
};

class while_statement_t : public statement_t
{
private:
	expression_t * condition;
	statement_t * body;

	bool condition_true(code_block_t * block);

public:
	while_statement_t(expression_t * condition, statement_t * body);
	flow_interruption_type execute(code_block_t * block);
};

class break_statement_t : public statement_t
{
	flow_interruption_type execute(code_block_t * block);
};

class invoke_statement_t : public statement_t
{
private:
	expression_t * invokee;
public:
	invoke_statement_t(expression_t * invokee);
	flow_interruption_type execute(code_block_t * block);
};

// Utilities

void raise_error(const char * format, ...);
variable_type parse_type(const char * string);
const char * to_string(variable_type type);
const char * to_string(variant_t value);
bool variant_equals(variant_t first, variant_t second);

#include "fortran.tab.hpp"