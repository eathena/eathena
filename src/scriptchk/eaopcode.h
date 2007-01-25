// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EAOPCODE_
#define _EAOPCODE_

///////////////////////////////////////////////////////////////////////////////
// command opcodes for stack interpreter
enum command_t
{
	/////////////////////////////////////////////////////////////////
	// "no operation" as usual
	OP_NOP				=  0,	
	/////////////////////////////////////////////////////////////////
	// assignment operations
	// take two stack values and push up one
	OP_ASSIGN			=  1,	// <Op If> '='   <Op>
	/////////////////////////////////////////////////////////////////
	// logic operations
	// take two stack values and push a value
	OP_BIN_AND			=  2,	// <Op BinAND> '&' <Op Equate>
	OP_BIN_OR			=  3,	// <Op BinOr> '|' <Op BinXOR>
	OP_BIN_XOR			=  4,	// <Op BinXOR> '^' <Op BinAND>
	/////////////////////////////////////////////////////////////////
	// compare operations
	// take two stack values and push a boolean value
	OP_EQUATE			=  5,	// <Op Equate> '==' <Op Compare>
	OP_UNEQUATE			=  6,	// <Op Equate> '!=' <Op Compare>
	OP_ISGT				=  7,	// <Op Compare> '>'  <Op Shift>
	OP_ISGTEQ			=  8,	// <Op Compare> '>=' <Op Shift>
	OP_ISLT				=  9,	// <Op Compare> '<'  <Op Shift>
	OP_ISLTEQ			= 10,	// <Op Compare> '<=' <Op Shift>
	/////////////////////////////////////////////////////////////////
	// shift operations
	// take two stack values and push a value
	OP_LSHIFT			= 11,	// <Op Shift> '<<' <Op AddSub>
	OP_RSHIFT			= 12,	// <Op Shift> '>>' <Op AddSub>
	/////////////////////////////////////////////////////////////////
	// add/sub operations
	// take two stack values and push a value
	OP_ADD				= 13,	// <Op AddSub> '+' <Op MultDiv>
	OP_SUB				= 14,	// <Op AddSub> '-' <Op MultDiv>
	/////////////////////////////////////////////////////////////////
	// mult/div/modulo operations
	// take two stack values and push a value
	OP_MUL				= 15,	// <Op MultDiv> '*' <Op Unary>
	OP_DIV				= 16,	// <Op MultDiv> '/' <Op Unary>
	OP_MOD				= 17,	// <Op MultDiv> '%' <Op Unary>
	/////////////////////////////////////////////////////////////////
	// unary operations
	// take one stack values and push a value
	OP_NEGATE			= 18,	// '-'    <Op Unary>
	OP_INVERT			= 19,	// '~'    <Op Unary>
	OP_NOT				= 20,	// '!'    <Op Unary>
	/////////////////////////////////////////////////////////////////
	// sizeof operations
	// take one stack values and push the result
	OP_SIZEOF			= 21,	// sizeof '(' Id ')'
	// sizeof '(' <Type> ')' replaces with OP_PUSH_INT on compile time
	/////////////////////////////////////////////////////////////////
	// cast operations
	// take one stack values and push the result
	OP_CAST				= 22,	// '(' <Type> ')' <Op Unary>   !CAST
	/////////////////////////////////////////////////////////////////
	// Pre operations
	// take one stack variable and push a value
	OP_PREADD			= 23,	// '++'   <Op Pointer>
	OP_PRESUB			= 24,	// '--'   <Op Pointer>
	/////////////////////////////////////////////////////////////////
	// Post operations
	// take one stack variable and push a value
	OP_POSTADD			= 25,	// <Op Pointer> '++'
	OP_POSTSUB			= 26,	// <Op Pointer> '--'
	/////////////////////////////////////////////////////////////////
	// Member Access
	// take a variable and a value from stack and push a varible
	OP_MEMBER			= 27,	// <Op Pointer> '.' <Value>
	OP_SCOPE			= 28,	// <Op Pointer> '::' <Value>
	/////////////////////////////////////////////////////////////////
	// standard function calls
	// check the parameters on stack before or inside the call of function
	OP_FUNCTION			= 29,	// followed by an string address and a byte for parameter count
//
//
//
	OP_SUBFUNCTION		= 29,	// followed by an address and a byte for parameter count
//
//
//
	/////////////////////////////////////////////////////////////////
	// explicit stack pushes
	// Values pushed on stack directly
	OP_PUSH_NONE		= 30,	//
	OP_PUSH_ADDR		= 31,	// followed by an address
//
//
//
	OP_PUSH_INT1		= 32,	// followed by an integer 1byte
	OP_PUSH_INT2		= 33,	// followed by an integer 2byte
	OP_PUSH_INT3		= 34,	// followed by an integer 3byte
	OP_PUSH_INT4		= 35,	// followed by an integer 4byte
//
//
//
	OP_PUSH_INT8		= 36,	// followed by an integer 8byte
	OP_PUSH_STRING		= 37,	// followed by an string address
//
//
//
	OP_PUSH_FLOAT		= 38,	// followed by a splitted float 4 byte
	OP_PUSH_VAR			= 39,	// followed by an string address
//
//
//
	OP_PUSH_VAL			= 40,	// followed by an string address
//
//
//
	OP_PUSH_PARAVAR		= 41,	// followed by an integer 1byte
//
	OP_PUSH_PARAVAL		= 42,	// followed by an integer 1byte
//
	OP_PUSH_TEMPVAR1	= 43,	// followed by the number of the temp variable 1byte
	OP_PUSH_TEMPVAR2	= 44,	// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAL1	= 45,	// followed by the number of the temp variable 1byte 
	OP_PUSH_TEMPVAL2	= 46,	// followed by the number of the temp variable 2byte
	/////////////////////////////////////////////////////////////////
	// array operations
	// take a variable and a value from stack and push a varible
	OP_ARRAY			= 47,	// <Op Pointer> '[' <Expr> ']'
	OP_ARRAYSEL			= 48,	// <Op Pointer> '[' <Expr List> ']'
	OP_RANGE			= 49,	// <Op Pointer> '[' <Expr>':'<Expr> ']'
	OP_SPLICE			= 50,	// <Op Pointer> '[' <Expr>':'<Expr>':'<Expr> ']'
	OP_DULICATE			= 51,	// <Op Pointer> '[' <Expr>':'<Expr>':*'<Expr> ']'
	// creation and modification
	OP_CONCAT1			= 52,	// followed by the number of array elements 1byte 
	OP_CONCAT2			= 53,	// followed by the number of array elements 2byte
	OP_CONCAT3			= 54,	// followed by the number of array elements 3byte
	OP_CONCAT4			= 55,	// followed by the number of array elements 4byte
	OP_CREATEARRAY		= 56,	// followed by dimension (char), create a empty var array
	/////////////////////////////////////////////////////////////////
	// maintainance
	OP_CLEAR			= 57,	// clear a variable
	OP_POP				= 58,	// clear the stack
	OP_EVAL				= 59,	// evaluate
	OP_BOOLEAN			= 60,	// convert to boolean
	/////////////////////////////////////////////////////////////////
	// markers
	OP_START			= 61,	// Program Start followed by 3byte Programm length
	OP_END				= 62,	// Quit the interpreter immediately
	OP_RETURN			= 63,	// return, quit if last scope	
	/////////////////////////////////////////////////////////////////
	// Jumps
	// conditional branch
	OP_NIF				= 64,	// branch when false, followed by the target address
	OP_IF				= 65,	// branch when true,  followed by the target address
	OP_NIF_POP			= 66,	// branch when false, pop when true, followed by the target address
	OP_IF_POP			= 67,	// branch when true, pop when false, followed by the target address
	// unconditional branch
	OP_GOTO				= 68,	// goto Id ';' followed by the target address
//
//
//
	OP_GOSUB			= 69	// gosub Id ';' followed by the target address
//
//
//
};

#endif//_EAOPCODE_
