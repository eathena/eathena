
#include "scriptprn.h"

///////////////////////////////////////////////////////////////////////////////
char unescape(const char *& str)
{
	if(*str=='\\')
	{
		++str;
		switch(*str)
		{
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case '\0':
			return *(--str);
		}
	}
	return *str;
}

void str2id(char*id, size_t sz, const char*str)
{
	char *ip=id;
	if(str)
	{	// convert to id (\w[\w\d]*)
		for(; *str; ++str)
		{
			if(ip<id+sz-1)
			{	
				const char c = unescape(str);
				*ip = ((ip==id)?basics::stringcheck::isalpha(c):basics::stringcheck::isalnum(c))?c:'_';
				++ip;
			}
		}
	}
	if(ip) *ip=0;
}

void str2name(char*name, size_t sz, const char*str)
{
	char *ip=name;
	if(str)
	{	// ignore controls, replace quotes with escapes
		for(; *str; ++str)
		{
			if( ip<name+sz-1 && !basics::stringcheck::iscntrl(*str) )
			{	
				if('"'==*str)
					*ip++ = '\\';
				*ip++=*str;
			}		
		}
	}
	if(ip) *ip=0;
}
void str2strip_quotes(char*target, size_t sz, const char*str)
{
	char*ip = target;
	if(str)
	{	
		if(*str=='\"') ++str;
		for(; *str && (str[0]!='\"' || str[1]) && (ip<target+sz-1); ++str)
		{
			if( !basics::stringcheck::iscntrl(*str) )
			{	
				if('"'==*str)
					*ip++ = '\\';
				*ip++ = *str;
			}
		}
	}
	if(ip) *ip=0;
}


///////////////////////////////////////////////////////////////////////////////

void printer::put(const char c)
{	// ignore carriage return
	if( c!='\r' )
	{
		if(this->newline && !ignore_nl)
		{
			size_t i;
			for(i=0; i<scope; ++i)
				fputc('\t', this->output);
		}
		this->newline = (c=='\n');
		fputc( (this->newline && ignore_nl)?' ':c, this->output);
	}
}
void printer::put(const char *str)
{
	if(str)
	{
		for(; *str; ++str)
			this->put(*str);
	}
}

void printer::print_id(const char* str)
{
	if(str)
	{	// convert to id (\w[\w\d]*)
		printer& prn = *this;
		const char c = unescape(str);
		prn << (basics::stringcheck::isalpha(c)?c:'_');
		for(++str; *str; ++str)
		{	
			const char c = unescape(str);
			prn << (basics::stringcheck::isalnum(c)?c:'_');
		}
	}
}

void printer::print_name(const char* str)
{
	if(str)
	{	// ignore controls, replace quotes with escapes
		printer& prn = *this;
		for(; *str; ++str)
		{
			if( !basics::stringcheck::iscntrl(*str) )
			{	
				if('"'==*str)
					prn << '\\';
				prn << *str;
			}		
		}
	}
}

void printer::print_without_quotes(const char* str)
{
	if(str)
	{	
		printer& prn = *this;
		if(*str=='\"') ++str;
		for(; *str && (str[0]!='\"' || str[1]); ++str)
		{
			if( !basics::stringcheck::iscntrl(*str) )
			{	
				if('"'==*str)
					prn << '\\';
				prn << *str;
			}		
		}
	}
}

void printer::print_comments(basics::CParser_CommentStore& parser, int rtpos)
{
	printer& prn = *this;

	// go down the first node until reaching a terminal
	size_t linelimit = static_cast<size_t>(-1);

	if( rtpos >= 0)
	{
		while( parser.rt[rtpos].symbol.Type == 0 && parser.rt[rtpos].cChildNum )
		{
			rtpos = parser.rt[rtpos].cChildPos;	// select the first child
		}
		linelimit = parser.rt[rtpos].cToken.line;
	}

	if( rtpos < 0 || parser.rt[rtpos].symbol.Type == 1 )
	{	// reached a terminal
		// print all coments up to this line

		// print comments
		while( parser.cCommentList.size() )
		{
			if( parser.cCommentList[0].line <= linelimit )
			{
				if(!prn.newline) prn << '\n';
				prn << ((parser.cCommentList[0].multi)?"/*":"// ") 
					<< parser.cCommentList[0].content
					<< ((parser.cCommentList[0].multi)?"*/\n":"\n");
				parser.cCommentList.removeindex(0);
			}
			else
				break;
		}
	}
}


int printer::log(const char*fmt, ...)
{
	if(logfile)
	{
		va_list argptr;
		va_start(argptr, fmt);
		int ret = vfprintf(logfile, fmt, argptr);
		fputc('\n', logfile);
		va_end(argptr);
		return ret;
	}
	return 0;
}

static int internlog(FILE*logfile, basics::CParser_CommentStore& parser, int rtpos)
{
	int ret = 0;
	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// print the ternimal
		const char* str = parser.rt[rtpos].cToken.cLexeme;
		ret += fprintf(logfile, str);
	}
	else
	{
		size_t j=  parser.rt[rtpos].cChildPos;
		size_t k=j+parser.rt[rtpos].cChildNum;
		for(; j<k; ++j)
		{	
			if( parser.rt[j].symbol.Type == 1 )
			{	// print the ternimal
				const char* str = parser.rt[j].cToken.cLexeme;
				ret += fprintf(logfile, str);
				fputc(' ', logfile), ++ret;
			}
			else
			{	// go down
				ret += internlog(logfile, parser, j);
			}
		}
	}
	return ret;
}

int printer::log(basics::CParser_CommentStore& parser, int rtpos)
{
	if(logfile)
	{
		int ret = internlog(logfile,parser,rtpos);
		if(ret>0) fputc('\n',logfile);
		return ret;
	}
	return 0;
}
