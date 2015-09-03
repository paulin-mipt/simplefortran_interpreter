#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>

enum variable_type
{
	vtInt,
	vtBool,
	vtNoType
};

enum operation
{
	opAdd,
	opSub,
	opMul,
	opDiv,
	opMod,

	opAnd,
	opOr,
	opNot, 

	opEquals,
	opNotEquals,
	opLesser,
	opGreater,
	opLesserEquals,
	opGreaterEquals
};

enum flow_interruption_type
{
	fitReturn,
	fitBreak,
	fitNoIterruption
};

struct variant_t
{
	variable_type type;
	union
	{
		bool bool_value;
		int int_value;
	};
};

struct variable_t
{
	variant_t value;
	bool is_assigned;
	bool is_declared;
	
	variable_t()
	{
		is_assigned = false;
		is_declared = false;
		value.type = vtNoType;
	}

	bool is_assignable_from(variable_type type)
	{
		return type == value.type;
	}

	void set_value(variant_t value)
	{
		if (!is_assignable_from(value.type))
		{
			return;
		}

		this->value = value;
	}

	void set_type(variable_type type)
	{
		if (value.type != vtNoType)
		{
			return;
		}

		value.type = type;
	}

	variable_type get_type()
	{
		return value.type;
	}
};