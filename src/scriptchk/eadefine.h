// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EADEFINE_
#define _EADEFINE_

#include "basestring.h"


///////////////////////////////////////////////////////////////////////////////
/// map of define statements.
struct scriptdefines
{
	basics::smap<basics::string<>, basics::variant> table;
	typedef basics::smap<basics::string<>, basics::variant>::iterator iterator;

	scriptdefines()	{}
	~scriptdefines()	{}

	// default copy/assignment

	bool exists(const basics::string<>& name) const
	{
		return this->table.exists(name);
	}
	basics::variant get(const basics::string<>& name) const
	{
		const basics::variant* ptr = this->table.search(name);
		if( ptr )
			return *ptr;
		return basics::variant();
	}
	void clear()
	{
		this->table.clear();
	}
	void insert(const basics::string<>& name, const basics::variant& var)
	{
		if( var.is_valid() )
		{	// add entry
			basics::variant& obj = this->table[name];
			if( !obj.is_empty() && obj != var )
			{	// have a redefinition
				printf("redefinition of '%s' ", name.c_str());
				printf("from '%s' ", (const char*)obj.get_string());
				printf("to '%s'\n",(const char*)var.get_string());
			}
			obj = var;
		}
		else
		{	// delete entry
			this->table.erase(name);
		}
	}
	void insert(const basics::string<>& name, const basics::string<>& var)
	{
		this->insert(name, basics::variant(var));
	}
	void insert(const basics::string<>& name, const int var)
	{
		this->insert(name, basics::variant(var));
	}
	void insert(const basics::string<>& name, const int64 var)
	{
		this->insert(name, basics::variant(var));
	}
	void insert(const basics::string<>& name, const double var)
	{
		this->insert(name, basics::variant(var));
	}
	const scriptdefines& operator+=(const scriptdefines& other)
	{
		basics::smap<basics::string<>, basics::variant>::iterator iter(other.table);
		for(; iter; ++iter)
		{
			this->insert(iter->key, iter->data);
		}
		return *this;
	}
};




///////////////////////////////////////////////////////////////////////////////
#endif//_EADEFINE_
