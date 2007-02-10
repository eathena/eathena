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
	OP_NOP=0,					
	/////////////////////////////////////////////////////////////////
	// assignment operations
	// take two stack values and push up one
	OP_ASSIGN,					// <Op If>  '='  <Op>
	OP_ARRAY_ASSIGN,			// <Concat> '='  <Op>
	OP_ADD_ASSIGN,				// <Op If> '+='  <Op>
	OP_SUB_ASSIGN,				// <Op If> '-='  <Op>
	OP_MUL_ASSIGN,				// <Op If> '*='  <Op>
	OP_DIV_ASSIGN,				// <Op If> '/='  <Op>
	OP_MOD_ASSIGN,				// <Op If> '%='  <Op>
	OP_BIN_XOR_ASSIGN,			// <Op If> '^='  <Op>
	OP_BIN_AND_ASSIGN,			// <Op If> '&='  <Op>
	OP_BIN_OR_ASSIGN,			// <Op If> '|='  <Op>
	OP_RSHIFT_ASSIGN,			// <Op If> '>>=' <Op>
	OP_LSHIFT_ASSIGN,			// <Op If> '<<=' <Op>
	/////////////////////////////////////////////////////////////////
	// add/sub operations
	// take two stack values and push a value
	OP_ADD,						// <Op AddSub> '+' <Op MultDiv>
	OP_SUB,						// <Op AddSub> '-' <Op MultDiv>
	/////////////////////////////////////////////////////////////////
	// mult/div/modulo operations
	// take two stack values and push a value
	OP_MUL,						// <Op MultDiv> '*' <Op Unary>
	OP_DIV,						// <Op MultDiv> '/' <Op Unary>
	OP_MOD,						// <Op MultDiv> '%' <Op Unary>
	/////////////////////////////////////////////////////////////////
	// logic operations
	// take two stack values and push a value
	OP_BIN_AND,					// <Op BinAND> '&' <Op Equate>
	OP_BIN_OR,					// <Op BinOr>  '|' <Op BinXOR>
	OP_BIN_XOR,					// <Op BinXOR> '^' <Op BinAND>
	/////////////////////////////////////////////////////////////////
	// shift operations
	// take two stack values and push a value
	OP_LSHIFT,					// <Op Shift> '<<' <Op AddSub>
	OP_RSHIFT,					// <Op Shift> '>>' <Op AddSub>
	/////////////////////////////////////////////////////////////////
	// compare operations
	// take two stack values and push a boolean value
	OP_EQUATE,					// <Op Equate>  '==' <Op Compare>
	OP_UNEQUATE,				// <Op Equate>  '!=' <Op Compare>
	OP_ISGT,					// <Op Compare> '>'  <Op Shift>
	OP_ISGTEQ,					// <Op Compare> '>=' <Op Shift>
	OP_ISLT,					// <Op Compare> '<'  <Op Shift>
	OP_ISLTEQ,					// <Op Compare> '<=' <Op Shift>
	/////////////////////////////////////////////////////////////////
	// unary operations
	// take one stack values and push a value
	OP_NEGATE,					// '-'    <Op Unary>
	OP_INVERT,					// '~'    <Op Unary>
	OP_NOT,						// '!'    <Op Unary>
	/////////////////////////////////////////////////////////////////
	// sizeof operations
	// take one stack values and push the result
	OP_SIZEOF,					// sizeof '(' Id ')'
	// sizeof '(' <Type> ')' replaces with OP_PUSH_INT on compile time
	/////////////////////////////////////////////////////////////////
	// cast operations
	// take one stack values and push the result
	OP_CAST_INTEGER,			// '(' <Type> ')' <Op Unary>
	OP_CAST_STRING,				// '(' <Type> ')' <Op Unary>
	OP_CAST_FLOAT,				// '(' <Type> ')' <Op Unary>

	/////////////////////////////////////////////////////////////////
	// Pre operations
	// take one stack variable and push a value
	OP_PREADD,					// '++'   <Op Pointer>
	OP_PRESUB,					// '--'   <Op Pointer>
	/////////////////////////////////////////////////////////////////
	// Post operations
	// take one stack variable and push a value
	OP_POSTADD,					// <Op Pointer> '++'
	OP_POSTSUB,					// <Op Pointer> '--'
	/////////////////////////////////////////////////////////////////
	// Member Access
	// take a variable and a value from stack and push a varible
	OP_MEMBER,					// <Op Pointer> '.' <Value>
	OP_SCOPE,					// <Op Pointer> '::' <Value>
	/////////////////////////////////////////////////////////////////
	// standard function calls
	// check the parameters on stack before or inside the call of function
	OP_FUNCTION,				// followed by an string address
	OP_FUNCTION2,				// followed by an string address
	OP_FUNCTION3,				// followed by an string address
	OP_FUNCTION4,				// followed by an string address
	OP_BLDFUNCTION,				// followed by an string address
	OP_BLDFUNCTION2,			// followed by an string address
	OP_BLDFUNCTION3,			// followed by an string address
	OP_BLDFUNCTION4,			// followed by an string address
	OP_SUBFUNCTION,				// followed by an string address
	OP_SUBFUNCTION2,			// followed by an string address
	OP_SUBFUNCTION3,			// followed by an string address
	OP_SUBFUNCTION4,			// followed by an string address
	/////////////////////////////////////////////////////////////////
	// explicit stack pushes
	// Values pushed on stack directly
	OP_PUSH_NONE,				//
	OP_PUSH_ZERO,				//
	OP_PUSH_ONE,				//
	OP_PUSH_INT,				// followed by an integer 1byte
	OP_PUSH_INT2,				// followed by an integer 2byte
	OP_PUSH_INT3,				// followed by an integer 3byte
	OP_PUSH_INT4,				// followed by an integer 4byte
	OP_PUSH_INT5,				// followed by an integer 4byte
	OP_PUSH_INT6,				// followed by an integer 4byte
	OP_PUSH_INT7,				// followed by an integer 4byte
	OP_PUSH_INT8,				// followed by an integer 8byte
	OP_PUSH_STRING,				// followed by an string address
	OP_PUSH_STRING2,			// followed by an string address
	OP_PUSH_STRING3,			// followed by an string address
	OP_PUSH_STRING4,			// followed by an string address
	OP_PUSH_FLOAT,				// followed by a splitted float 4 byte
	OP_PUSH_VAR,				// followed by an string address
	OP_PUSH_VAR2,				// followed by an string address
	OP_PUSH_VAR3,				// followed by an string address
	OP_PUSH_VAR4,				// followed by an string address
	OP_PUSH_VAL,				// followed by an string address
	OP_PUSH_VAL2,				// followed by an string address
	OP_PUSH_VAL3,				// followed by an string address
	OP_PUSH_VAL4,				// followed by an string address
	OP_PUSH_PARAVAR,			// followed by an integer 1byte
	OP_PUSH_PARAVAR2,			// followed by an integer 1byte
	OP_PUSH_PARAVAR3,			// followed by an integer 1byte
	OP_PUSH_PARAVAR4,			// followed by an integer 1byte
	OP_PUSH_PARAVAL,			// followed by an integer 1byte
	OP_PUSH_PARAVAL2,			// followed by an integer 1byte
	OP_PUSH_PARAVAL3,			// followed by an integer 1byte
	OP_PUSH_PARAVAL4,			// followed by an integer 1byte
	OP_PUSH_TEMPVAR,			// followed by the number of the temp variable 1byte
	OP_PUSH_TEMPVAR2,			// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAR3,			// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAR4,			// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAL,			// followed by the number of the temp variable 1byte 
	OP_PUSH_TEMPVAL2,			// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAL3,			// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAL4,			// followed by the number of the temp variable 2byte
	/////////////////////////////////////////////////////////////////
	// array operations
	// take a variable and a value from stack and push a varible
	OP_ARRAY,					// <Op Pointer> '[' <Expr> ']'
	OP_ARRAYSEL,				// <Op Pointer> '[' <Expr List> ']'
	OP_ARRAYSEL2,				// <Op Pointer> '[' <Expr List> ']'
	OP_ARRAYSEL3,				// <Op Pointer> '[' <Expr List> ']'
	OP_ARRAYSEL4,				// <Op Pointer> '[' <Expr List> ']'
	OP_RANGE,					// <Op Pointer> '[' <Expr>':'<Expr> ']'
	OP_SPLICE,					// <Op Pointer> '[' <Expr>':'<Expr>':'<Expr> ']'
	OP_DULICATE,				// <Op Pointer> '[' <Expr>':'<Expr>':*'<Expr> ']'
	// creation and modification
	OP_CONCAT,					// followed by the number of array elements 1byte 
	OP_CONCAT2,					// followed by the number of array elements 2byte
	OP_CONCAT3,					// followed by the number of array elements 3byte
	OP_CONCAT4,					// followed by the number of array elements 4byte
	OP_CREATEARRAY,				// followed by dimension (char), create a empty var array
	/////////////////////////////////////////////////////////////////
	// maintainance
	OP_EMPTY,					// clear a variable
	OP_POP,						// clear the stack
	OP_EVAL,					// evaluate
	OP_BOOLEAN,					// convert to boolean
	/////////////////////////////////////////////////////////////////
	// Jumps
	// conditional branch
	OP_NIF,						// branch when false, followed by the target address
	OP_NIF2,					// branch when false, followed by the target address
	OP_NIF3,					// branch when false, followed by the target address
	OP_NIF4,					// branch when false, followed by the target address
	OP_IF,						// branch when true,  followed by the target address
	OP_IF2,						// branch when true,  followed by the target address
	OP_IF3,						// branch when true,  followed by the target address
	OP_IF4,						// branch when true,  followed by the target address
	OP_NIF_POP,					// branch when false, pop when true, followed by the target address
	OP_NIF_POP2,				// branch when false, pop when true, followed by the target address
	OP_NIF_POP3,				// branch when false, pop when true, followed by the target address
	OP_NIF_POP4,				// branch when false, pop when true, followed by the target address
	OP_IF_POP,					// branch when true, pop when false, followed by the target address
	OP_IF_POP2,					// branch when true, pop when false, followed by the target address
	OP_IF_POP3,					// branch when true, pop when false, followed by the target address
	OP_IF_POP4,					// branch when true, pop when false, followed by the target address
	// unconditional branch
	OP_GOTO,					// goto Id ';' followed by the target address
	OP_GOTO2,					// goto Id ';' followed by the target address
	OP_GOTO3,					// goto Id ';' followed by the target address
	OP_GOTO4,					// goto Id ';' followed by the target address
	OP_GOSUB,					// gosub Id ';' followed by the target address
	OP_GOSUB2,					// gosub Id ';' followed by the target address
	OP_GOSUB3,					// gosub Id ';' followed by the target address
	OP_GOSUB4,					// gosub Id ';' followed by the target address
	/////////////////////////////////////////////////////////////////
	// markers
	OP_RETURN,					// return, quit if last scope	
	OP_END,						// quit the interpreter immediately
	OP_START					// Program Start followed by 3byte Programm length
};

#endif//_EAOPCODE_
