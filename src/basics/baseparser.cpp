#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"
#include "baseparser.h"


NAMESPACE_BEGIN(basics)

//
// Action Types
//
#define ActionShift		1
#define ActionReduce	2
#define ActionGoto		3
#define ActionAccept	4



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Reduction Tree functions
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CParser::print_stack_element(const CStackElement &se, int indent, const char sign) const
{
	int i;
	printf("(%3i/%4i)", se.cToken.line, se.cToken.column);
	for(i=0;i<indent;++i)
		printf("|%c",sign);
	printf("%c-<%s>(%i)[%li] ::= %s (len=%li)\n", (se.symbol.Type)?'T':'N', (const char*)se.symbol.Name, se.symbol.idx, (unsigned long)se.cChildNum, (const char*)se.cToken.cLexeme, (unsigned long)(((const char*)se.cToken.cLexeme)?strlen(se.cToken.cLexeme):0));
}
void CParser::print_rt_tree(int rtpos, int indent, bool trim) const
{
	int callindent = indent;

	if( !trim || this->rt[rtpos].cChildNum> 1 )
	{	
		callindent++;
		print_stack_element(this->rt[rtpos], indent, ' ');
	}
	if( this->rt[rtpos].cChildNum>0 )
	{
		size_t j,k=this->rt[rtpos].cChildPos+this->rt[rtpos].cChildNum;
		for(j=this->rt[rtpos].cChildPos; j<k; ++j)
		{
			if (this->rt[j].symbol.Type != 1) // non terminal
				this->print_rt_tree(j, callindent, trim);
			else
			{
				this->print_stack_element( this->rt[j], callindent, '-');
			}
		}
	}
}
void CParser::print_rt() const
{
	size_t i, k;
	printf ("print rt\n");
	for(i=0; i<this->rt.size(); ++i)
	{
		printf("%03li - (%i) %s, childs:", (unsigned long)i, this->rt[i].symbol.idx, (const char*)this->rt[i].symbol.Name);

		for(k=0; k<this->rt[i].cChildNum; ++k)
			printf("%li ", (unsigned long)(this->rt[i].cChildPos+k));
		printf("\n");
	}
}

void CParser::print_stack() const
{
	size_t i,k;
printf ("print stack\n");
	for(i=0; i<this->cStack.size(); ++i)
	{
		printf("%03li - (%i) %s, childs:", (unsigned long)i, this->cStack[i].symbol.idx, (const char*)this->cStack[i].symbol.Name);

		for(k=0; k<this->rt[i].cChildNum; ++k)
			printf("%li ", (unsigned long)(this->rt[i].cChildPos+k));

		printf("\n");
	}
}
void CParser::print_expects(const char*name) const
{
	fprintf(stderr, "Parse Error at line %i, col %i\n", this->cScanToken.line, this->cScanToken.column);
	if(name&&*name) fprintf(stderr, "in file '%s'\n", name);

	if(this->cScanToken.id >=0)
	{
		fprintf(stderr,"recognized: %s '%s'\n", 
			(const char*) pconfig->sym[this->cScanToken.id].Name,
			(const char*)this->cScanToken.cLexeme);
	}
	else
	{
		fprintf(stderr,"unrecognized token: '%c'\n", 
			const_cast<CParser*>(this)->input.get_char());
	}
	if(this->lalr_state>=0)
	{
		size_t i;
		fprintf(stderr,"expecting: ");
		for(i=0; i<this->pconfig->lalr_state[this->lalr_state].cAction.size(); ++i)
		{
			CAction* action = &this->pconfig->lalr_state[this->lalr_state].cAction[i];
			fprintf(stderr,pconfig->sym[action->SymbolIndex].Type==1?"'%s' ":"<%s> ", 
				(const char*) pconfig->sym[action->SymbolIndex].Name);
		}
	}
	fprintf(stderr,"\n");
}








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Scanner functions
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Default scanner match function
///////////////////////////////////////////////////////////////////////////////
bool CParser::DefaultMatchFunction(short type, const string<>& name, short symbol)
{
	switch(type)
	{
	case SymbolTypeWhitespace:
		return false;
	default:
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// This function is called when the scanner matches a token
//
// Our implementation handles the "Comment Start" symbol, and scans through
// the input until the token closure is found
//
// All other tokens are refered to the default match function
///////////////////////////////////////////////////////////////////////////////
bool CParser::MatchFunction(short type, const string<>& name, short symbol)
{
	short c;
	switch(type) {
	case SymbolTypeCommentLine: // ;
	{
		c = this->input.get_char();
		while( c != EEOF )
		{
			if(c == 13 || c == 10)
				break;

			this->input.next_char();
			c = this->input.get_char();
		}
/*		c = this->input.get_charstep();
		while( c != EEOF )
		{
			if(c == 13 || c == 10)
			{	// skip all following returns
				// note that nextchar/getchar are used here
				c = this->input.get_char();
				while (c == 13 || c == 10)
				{
					this->input.next_char();
					c = this->input.get_char();
				}
				break;
			}
			c = this->input.get_charstep();
		}
*/
		return false;
	}
	case SymbolTypeCommentStart: // /* */
	{
		c = this->input.get_charstep();
		while( c != EEOF )
		{
			if (c == '*')
			{
				c = this->input.get_charstep();
				if(c == '/')
					break;
			}
			else
			{
				c = this->input.get_charstep();
			}
		}
		return false;
	}
	default:
		return this->DefaultMatchFunction(type,name,symbol);
	}
}
bool CParser_CommentStore::MatchFunction(short type, const string<>& name, short symbol)
{
	short c;
	switch(type) {
	case SymbolTypeCommentLine: // ;
	{
		size_t line = this->input.line;
		string<> str;

		c = this->input.get_char();
		while( c != EEOF )
		{
			if(c == 13 || c == 10)
				break;
			str.append( (char)c );
			this->input.next_char();
			c = this->input.get_char();
		}
/*
		c = this->input.get_charstep();
		while( c != EEOF )
		{
			if(c == 13 || c == 10)
			{
				// skip all following returns
				// note that nextchar/getchar are used here
				c = this->input.get_char();
				while (c == 13 || c == 10)
				{
					this->input.next_char();
					c = this->input.get_char();
				}
				break;
			}
			str.append( (char)c );

			c = this->input.get_charstep();
		}
*/
		str.trim();
		cCommentList.append( CLineStorage(line, str) );
		return false;
	}
	case SymbolTypeCommentStart: // / * * /
	{
		size_t line = this->input.line;
		string<> str;

		c = this->input.get_charstep();
		while( c != EEOF )
		{
			if (c == '*')
			{
				c = this->input.get_charstep();
				if(c == '/')
					break;
				else
					str.append('*');
			}
			else
			{
				str.append( (char)c );
				c = this->input.get_charstep();
			}
		}
		cCommentList.append( CLineStorage(line, str, true) );
		return false;
	}
	default:
		return this->DefaultMatchFunction(type,name,symbol);
	}
}



short CParseInput::scan(CParser& parser, CToken& target)
{
	int c=0;
	int idx;
	const CDFAState* dfa = &parser.pconfig->dfa_state[parser.pconfig->init_dfa];
	int last_accepted=-1;
	int last_accepted_size=0;
	int last_accepted_line=this->line;
	int last_accepted_col=this->column;

	target.clear();
	target.line   = this->line;
	target.column = this->column;

	// check for eof
	if( this->get_eof(false) )
		return 0;

	while(1)
	{
		int i=0;
		int nedge=0;
		// get char from input stream
		c = this->get_charstep();

		// convert to lower case
		if(!parser.pconfig->case_sensitive && c != EEOF)
			c = stringcheck::tolower( (char)c );

		// look for a matching edge
		if (c != EEOF)
		{
			nedge = dfa->cEdge.size();
			for (i=0; i<nedge; ++i)
			{
				idx = dfa->cEdge[i].CharSetIndex;
				if (strchr(parser.pconfig->charset[idx], c))
				{
					dfa = &parser.pconfig->dfa_state[dfa->cEdge[i].TargetIndex];
					if (dfa->Accept)
					{
						last_accepted = dfa->AcceptIndex;
						last_accepted_size = (this->cScn-this->cRpp) ;// + 1;
						last_accepted_line=this->line;
						last_accepted_col=this->column;
					}
					break;
				}
			}
		}
		if( (c == EEOF) || (i == nedge) )
		{	// accept, ignore or invalid token

			if (last_accepted == -1)
			{	// invalid token, just reset the reader
				this->cScn=this->cRpp;
				this->line = last_accepted_line;
				this->column=last_accepted_col;
			}
			else //if (last_accepted != -1)
			{
				// copy the token
				target.cLexeme.assign( this->cRpp, last_accepted_size );
				// reset buffer counters to start right after the matched token
				this->cRpp+=last_accepted_size;
				this->cScn=this->cRpp;
				this->line = last_accepted_line;
				this->column=last_accepted_col;
			
				if( !parser.MatchFunction(parser.pconfig->sym[last_accepted].Type, parser.pconfig->sym[last_accepted].Name, (short)last_accepted) )
				{	// ignore, reset state
					target.clear();
					target.line   = this->line;
					target.column = this->column;

					if (c == EEOF || (last_accepted == -1))
						return 0;

					dfa = &parser.pconfig->dfa_state[parser.pconfig->init_dfa];
					last_accepted = -1;

					this->cRpp=this->cScn;
					continue;
				}
				this->cRpp=this->cScn;
			}
			break;
		}
	}
	if (last_accepted == -1)
	{	// invalid
		target.clear();
		if(c != EEOF) target.id = -1;
	}
	else
	{	// accept
		target.id=last_accepted;
	}
	return target.id;
}
/*
///////////////////////////////////////////////////////////////////////////////
// Scan input for next token
///////////////////////////////////////////////////////////////////////////////
short CParseInput::scan(CParser& parser, CToken& target)
{
	int c=0;
	int idx;
	int start_ofs = this->nofs;
	int last_accepted=-1;
	int last_accepted_size=0;
	int line=this->line, col=this->column;
	const CDFAState* dfa = &parser.pconfig->dfa_state[parser.pconfig->init_dfa];

	target.clear();
	target.line   = this->line;
	target.column = this->column;

	// check for eof
	if( this->get_eof(false) )
		return 0;

	while(1)
	{
		int i=0;
		int nedge=0;
		// get char from input stream
		c = this->get_char( (last_accepted == -1 || !dfa->Accept) );

		// save line/col of the last valid position
		if(dfa->Accept)
		{
			line=this->line;
			col=this->column;
		}

		// convert to lower case
		if (!parser.pconfig->case_sensitive && c != EEOF)
			c = stringcheck::tolower(c);

		// look for a matching edge
		if (c != EEOF) {
			nedge = dfa->cEdge.size();
			for (i=0; i<nedge; ++i) {
				idx = dfa->cEdge[i].CharSetIndex;
				if (strchr(parser.pconfig->charset[idx], c)) {
					dfa = &parser.pconfig->dfa_state[dfa->cEdge[i].TargetIndex];
					target.cLexeme.append( (char)c );
					if (dfa->Accept) {
						last_accepted = dfa->AcceptIndex;
						last_accepted_size = (this->nofs - start_ofs) + 1;
					}
					break;
				}
			}
		}
		if ((c == EEOF) || (i == nedge)) {
			// accept, ignore or invalid token
			if (last_accepted != -1)
			{
				// reset buffer counters to start right after the matched token
				this->line = line;
				this->column=col;
				if(last_accepted_size>0)
				{
					target.cLexeme.truncate(last_accepted_size);
					this->nofs = start_ofs+last_accepted_size;
				}
				else
					printf("Warning negative Lexeme Resize, skipping\n");
			
				if( !parser.MatchFunction(parser.pconfig->sym[last_accepted].Type, parser.pconfig->sym[last_accepted].Name, (short)last_accepted) )
				{	// ignore, reset state
					target.clear();
					target.line   = this->line;
					target.column = this->column;

					if (c == EEOF || (last_accepted == -1))
						return 0;

					dfa = &parser.pconfig->dfa_state[parser.pconfig->init_dfa];
					last_accepted = -1;
					start_ofs = this->nofs;
					continue;
				}
			}
			break;
		}
		// move to next character
		this->next_char();
	}
	if (last_accepted == -1)
	{	// invalid
		target.clear();
		if(c != EEOF) target.id = -1;
	}
	else
	{	// accept
		target.id=last_accepted;
	}
	return target.id;
}
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Parser functions
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Get symbol info
///////////////////////////////////////////////////////////////////////////////
bool CParser::get_symbol(size_t syminx, CSymbol& symbol) const
{
	if(syminx >= this->pconfig->sym.size())
		return false;
	symbol = this->pconfig->sym[syminx];
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Create a new parser
///////////////////////////////////////////////////////////////////////////////
bool CParser::create(CParseConfig* pcfg)
{	
	this->pconfig = pcfg;
	this->cStack.realloc(STACK_SIZE);

	this->lalr_state = pconfig->init_lalr;
	this->tokens.realloc(TOKEN_SIZE);

	// Reduction Tree
	this->rt.realloc(RT_BUFF_SIZE);
	this->rt.resize(1);// 0 is reserved as head
	// End Reduction Tree

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Reset the parser
///////////////////////////////////////////////////////////////////////////////
void CParser::reinit()
{	// basically clear stack and reduction tree
	// actually only the first stack element is the link to 
	// the current elements on the reduction tree
	// so deleting this stack element removes this link
	// but we need at least on of these elements on the stack 
	// because it contains the finalizing reduction rule
	// so we just empty the children list of the first and delete the others

	// find the reduced symbol (which is in rt[0])
	size_t i, c;
	for(i=0, c=0; i<this->cStack.size(); ++i)
	{
		if( this->rt[0].symbol.idx == this->cStack[i].symbol.idx &&
			this->cStack[i].cChildNum )
		{	// only clear the children
			this->cStack[i].cChildNum = 0;
			this->cStack[i].cChildPos = 0;
			break;
		}
	}
	// delete the complete reduction tree
	this->rt.resize(1);	// 0 is reserved as head
}


///////////////////////////////////////////////////////////////////////////////
// Reset the parser
///////////////////////////////////////////////////////////////////////////////
void CParser::reset()
{
	this->input.close();
	this->cStack.clear();
	this->tokens.clear();

	this->lalr_state = this->pconfig->init_lalr;
	this->reduce_rule = 0;
	this->nstackofs = 0;
	this->reduction = false;

	// Reduction Tree
	this->rt.clear();
	this->rt.resize(1);	// 0 is reserved as head
	// End Reduction Tree
}

///////////////////////////////////////////////////////////////////////////////
// Reduction Tree
///////////////////////////////////////////////////////////////////////////////
const CStackElement* CParser::get_rt_entry(size_t idx) const
{
	if(idx >= this->rt.size())
		return NULL;
	return &(this->rt[idx]);
}



///////////////////////////////////////////////////////////////////////////////
// Parse
///////////////////////////////////////////////////////////////////////////////
short CParser::parse(short reduce_sym)
{
	size_t i;
	char bfound;
	CParseInput* pinput = &this->input;

	if(this->reduction)
	{
		CRule* rule = &this->pconfig->rule[this->reduce_rule];
		size_t nrtIdx = rule->cSymbol.size();
		CSymbol tmpsym;
		this->get_symbol(rule->NonTerminal, tmpsym);
		
		// push onto token stack
		this->tokens.push( CToken(rule->NonTerminal) );

		if( this->cStack.size() < nrtIdx )
			nrtIdx = this->cStack.size();

		if( nrtIdx==1 && reduce_sym && tmpsym.idx != reduce_sym )
		{	// revert lalr_state
			this->lalr_state = this->cStack.last().state;
		}
		else
		{
			// prepare a new element for the stack a the target position
			// all elements behind will be poped from stack and pushed on the rt tree
			// and inserted as child elements in the new inserted node
			CStackElement se;

			if(nrtIdx)
			{	// remove terminals from stack
				// and move them onto reduction tree
				//this->rt.realloc(nrtIdx, RT_BUFF_SIZE);
				if( this->rt.size()+nrtIdx > this->rt.capacity() )
					this->rt.realloc(this->rt.size()+RT_BUFF_SIZE);

				se.cChildPos = this->rt.size();
				// move elements to rt
				for(i=this->cStack.size() - nrtIdx; i<this->cStack.size(); ++i)
					this->rt.push(this->cStack[i]);
				// remove them from stack
				cStack.resize(this->cStack.size() - nrtIdx);
				
				// revert lalr_state
				this->lalr_state = this->rt[se.cChildPos].state;
			}
			else
				se.cChildPos = 0;
			se.cChildNum = nrtIdx;

			// build the other data of the new nonterminal
			// get symbol information
			this->get_symbol(rule->NonTerminal, se.symbol);
			se.cToken    = this->cScanToken;
			se.cToken.id = rule->NonTerminal;
			se.state = this->lalr_state;
			se.rule = this->reduce_rule;

			cStack.push(se);
		}
		

		// Reduction tree head (always this->rt[0])
		this->rt[0] = this->cStack.last();

		this->reduction = false;
	}

	while(1)
	{
		if(this->tokens.size()<1)
		{	// No input tokens on stack, grab one from the input stream
			if( pinput->scan(*this, this->cScanToken) < 0 )
				return -1;

			this->tokens.push( this->cScanToken );
		}
		else
		{	// Retrieve the last token from the input stack
			size_t sz=this->tokens.size();
			if(sz>0)
			{	
				this->cScanToken = this->tokens[sz-1];
			}
		}

		bfound = 0;
		for (i=0; (!bfound) && (i<this->pconfig->lalr_state[this->lalr_state].cAction.size()); ++i)
		{
			CAction* action = &this->pconfig->lalr_state[this->lalr_state].cAction[i];
			if(action->SymbolIndex == this->cScanToken.id) {
				bfound = 1;
				switch(action->Action) {
				case ActionShift:
				{	// Push a symbol onto the stack
					CStackElement* se;
					size_t sz = this->cStack.size();

					//this->cStack.realloc(1,STACK_SIZE);
					if( this->cStack.size() >= this->cStack.capacity() )
						this->cStack.realloc(this->cStack.size()+STACK_SIZE);
					this->cStack.resize(sz+1);
					se = &this->cStack[sz];

					// get symbol information
					this->get_symbol(action->SymbolIndex, se->symbol);

					se->cToken    = this->cScanToken;
					se->cToken.id = action->SymbolIndex;

					se->state = this->lalr_state;
					se->rule = this->reduce_rule;
					se->cChildPos=0;
					se->cChildNum=0;

					this->nstackofs = this->cStack.size()-1;
					this->lalr_state = action->Target;
					// pop token from stack
					this->tokens.resize(this->tokens.size()-1);
					break;
				}
				case ActionReduce:
				{	// Reducing a rule is done in two steps:
					// 1] Setup the stack offset so the calling function
					//    can reference the rule's child lexeme values when
					//    this action returns
					// 2] When this function is called again, we will
					//    remove the child lexemes from the stack, and replace
					//    them with an element representing this reduction
					//
					CRule* rule = &this->pconfig->rule[action->Target];
					this->cScanToken.clear();

					this->reduce_rule = action->Target;
					if( this->cStack.size() > rule->cSymbol.size() )
						this->nstackofs = this->cStack.size() - rule->cSymbol.size();
					else
						this->nstackofs = 0;
					this->reduction = true;
					return rule->NonTerminal;
				}
				case ActionGoto:
				{	// Shift states
					this->lalr_state = action->Target;
					this->tokens.resize(this->tokens.size()-1);
					break;
				}
				case ActionAccept:
					// Eof, the main rule has been accepted
					return 0;
				} // switch
			} // if
		} // for
		if (!bfound)
		{
			if(this->cScanToken.id)
				break;
			return 0; // eof
		}
	} // while
	// token not found in rule
	return -1;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Parse Configuration Class
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Helper functions for loading .cgt files
///////////////////////////////////////////////////////////////////////////////
const unsigned char* getws(const unsigned char* b, char* s)
{
	while( (*s++ = *b++) ) ++b;
	++b; return b;
}
const unsigned char* getsh(const unsigned char* b, short* s)
{
	*s = *b++;
	*s |= (*b++) << 8;
	return b;
}
const unsigned char* getvws(const unsigned char* b, char* str)
{
	++b; return getws(b,str);
}
const unsigned char* skipvws(const unsigned char* b)
{
	++b; while(*b++) ++b;
	return ++b;
}
const unsigned char* getvb(const unsigned char* b, unsigned char* v)
{
	++b; *v = *b++;
	return b;
}
const unsigned char* getvsh(const unsigned char* b, short* v)
{
	++b; return getsh(b,v);
}

///////////////////////////////////////////////////////////////////////////////
// Load a parse table from memory
///////////////////////////////////////////////////////////////////////////////
bool CParseConfig::create(const unsigned char* b, size_t len)
{
	char str[65*1024];
	const unsigned char* bend;
	short nEntries;
	unsigned char recType;
	short idx;
	unsigned char byt;
	size_t i;

	if(!b || !len) return false;
	bend = b + len;

	// get header
	b = getws(b, str);

	// check header
	if(0!=strcmp(str, "GOLD Parser Tables/v1.0"))
		return false;

	// read records until eof
	while(b < bend)
	{
		b++; // skip record id

		// read number of entries in record
		b = getsh(b, &nEntries);

		// read record type
		b = getvb(b, &recType);

		switch(recType) {
		case 'P': // Parameters
		{
			b = skipvws(b); // Name
			b = skipvws(b); // Version
			b = skipvws(b); // Author
			b = skipvws(b); // About
			b = getvb(b, &byt); // Case Sensitive?
			b = getvsh(b, &this->start_symbol); // Start Symbol
			this->case_sensitive = byt?true:false;
			break;
		}
		case 'T': // Table Counts
		{
			short dummy;
			b = getvsh(b, &dummy);
			this->sym.resize(dummy);

			b = getvsh(b, &dummy);
			this->charset.resize(dummy);

			b = getvsh(b, &dummy);
			this->rule.resize(dummy);

			b = getvsh(b, &dummy);
			this->dfa_state.resize(dummy);
			
			b = getvsh(b, &dummy);
			this->lalr_state.resize(dummy);
			break;
		}
		case 'I': // Initial States
		{
			b = getvsh(b, &this->init_dfa);
			b = getvsh(b, &this->init_lalr);
			break;
		}
		case 'S': // Symbol Entry
		{
			b = getvsh(b, &idx);
			b = getvws(b, str);
			b = getvsh(b, &this->sym[idx].Type);
			this->sym[idx].Name = str;
			this->sym[idx].idx = idx;
			break;
		}
		case 'C': // Character Set Entry
		{
			b = getvsh(b, &idx);
			b = getvws(b, str);
			this->charset[idx] = str;
			break;
		}
		case 'R': // Rule Table Entry
		{
			b = getvsh(b, &idx);
			b = getvsh(b, &this->rule[idx].NonTerminal);
			b++; // reserved

			this->rule[idx].cSymbol.resize(((nEntries-4)>0)?(nEntries-4):0);
			for(i=0;i<this->rule[idx].cSymbol.size();++i)
				b = getvsh(b, &(this->rule[idx].cSymbol[i]));
			break;
		}
		case 'D': // DFA State Entry
		{
			b = getvsh(b, &idx);
			b = getvb(b, &byt);
			b = getvsh(b, &this->dfa_state[idx].AcceptIndex);
			this->dfa_state[idx].Accept = byt?1:0;
			b++; // reserved
			this->dfa_state[idx].cEdge.resize(((nEntries-5)/3)>0?((nEntries-5)/3):0);
			for (i=0; i<this->dfa_state[idx].cEdge.size(); ++i)
			{
				b = getvsh(b, &this->dfa_state[idx].cEdge[i].CharSetIndex);
				b = getvsh(b, &this->dfa_state[idx].cEdge[i].TargetIndex);
				b++; // reserved
			}
			break;
		}
		case 'L': // LALR State Entry
		{
			b = getvsh(b, &idx);
			b++; // reserved
			this->lalr_state[idx].cAction.resize((((nEntries-3)/4)>0)?((nEntries-3)/4):0);
			for (i=0;i<this->lalr_state[idx].cAction.size();++i)
			{
				b = getvsh(b, &this->lalr_state[idx].cAction[i].SymbolIndex);
				b = getvsh(b, &this->lalr_state[idx].cAction[i].Action);
				b = getvsh(b, &this->lalr_state[idx].cAction[i].Target);
				b++; // reserved
			}
			break;
		}
		default: // unknown record
			return false;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
// Load a parse table from a file
///////////////////////////////////////////////////////////////////////////////
bool CParseConfig::create(const char* filename)
{
	unsigned char* buf;
	FILE* fin;
	bool result=false;
#ifdef _WIN32
	struct _stat st;
	if (_stat(filename, &st) == -1) return false;
#else
	struct stat st;
	if (stat(filename, &st) == -1) return false;
#endif
	fin = fopen(filename, "rb");
	if(fin)
	{
		buf = new unsigned char[st.st_size];
		if(buf)
		{
			fread(buf, st.st_size, 1, fin);
			result = this->create(buf, st.st_size);
			delete[] buf;
		}
		fclose(fin);
	}
	return result;
}


NAMESPACE_END(basics)
