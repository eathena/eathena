#ifndef __BASEEXCEPTIONS_H__
#define __BASEEXCEPTIONS_H__

//////////////////////////////////////////////////////////////////////////
/// exceptions
//////////////////////////////////////////////////////////////////////////


NAMESPACE_BEGIN(basics)

//////////////////////////////////////////////////////////////////////////
/// basic exceptions class.
/// on windows there is already an exeption class in global namespace
/// when including any c++ header (same with string), 
/// so retreat to an own namespace.
/// low level cstring removes higher object dependency
class exception
{
protected:
    char* message;
public:
    explicit exception(const char* e) : message(e?strdup(e):NULL)
	{ }
    ~exception()				{ if(message) free(message); }
    const char* get_message()	{ return message?message:""; }
	operator const char *()		{ return this->get_message(); }
	const char *what()			{ return this->get_message(); }
};

//////////////////////////////////////////////////////////////////////////
/// exception for 'out of bound array access'.
class exception_bound : public exception
{
public:
	explicit exception_bound(const char*   e) : exception(e) {}
	~exception_bound()						{}
};

//////////////////////////////////////////////////////////////////////////
/// exception for 'memory allocation failed'.
class exception_memory : public exception
{
public:
	explicit exception_memory(const char*   e) : exception(e)	{}
	~exception_memory()							{}
};

//////////////////////////////////////////////////////////////////////////
/// exception for 'failed conversion'.
class exception_convert: public exception
{
public:
	explicit exception_convert(const char*   e) : exception(e)	{}
    ~exception_convert()						{}
};

//////////////////////////////////////////////////////////////////////////
/// variant exception class.
/// may be thrown when a variant is being typecasted to 32-bit int 
/// and the value is out of range
class exception_variant: public exception
{
public:
	explicit exception_variant(const char*   e) : exception(e)	{}
    ~exception_variant()						{}
};

//////////////////////////////////////////////////////////////////////////
/// exception for 'socket failed'
class exception_socket : public exception
{
public:
	explicit exception_socket(const char*   e) : exception(e)	{}
	~exception_socket()							{}
};


NAMESPACE_END(basics)

#endif//__BASEEXCEPTIONS_H__

