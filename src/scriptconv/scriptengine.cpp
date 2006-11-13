// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "scriptengine.h"
#include "basezlib.h"




const unsigned char* extractEngine(const unsigned char*buffer, unsigned long sourcesz, unsigned long &sz)
{
	static unsigned char buffer1[128*1024];
	basics::CZlib zlib;

	sz = sizeof(buffer1);
	if( 0==zlib.decode(buffer1, sz, buffer, sourcesz) )
	{
		fprintf(stderr, "loading engine...(%lu)\n", sz);
		return buffer1;
	}
	return NULL;
}

void buildEngine(const char*filename)
{
	static unsigned char buffer1[128*1024];
	static unsigned char buffer2[128*1024];
	FILE *fp = fopen(filename, "rb");
	ulong sz=fread(buffer1, 1, sizeof(buffer1), fp);

	ulong i, sz2=sizeof(buffer2);
	basics::CZlib zlib;
	zlib.encode(buffer2, sz2, buffer1, sz);

	for(i=0; i<sz2; ++i)
	{
		if(i%8==0) printf("\n");
		fprintf(stdout, "0x%02X, ", buffer2[i]);
	}
	exit(0);
}

bool parserstorage::init(const char* enginefile)
{
	if(parser_config) delete parser_config;
	if(parser) delete parser;
	if(enginefile)
	{
		try
		{
			this->parser_config = new basics::CParseConfig( enginefile );
		}
		catch(...)
		{
			this->parser_config=NULL;
		}
		if (!this->parser_config)
		{
			fprintf(stderr, "Could not open engine file %s\n", enginefile);
			return false;
		}
	}
	else
	{
		const unsigned char *buffer;
		ulong sz;
		this->getEngine(buffer, sz);
		const unsigned char *e = extractEngine(buffer, sz, sz);
		if(!e)
		{
			fprintf(stderr, "Error creating parser\n");
			return EXIT_FAILURE;
		}
		this->parser_config = new basics::CParseConfig(e, sz);
		if (!this->parser_config)
		{
			fprintf(stderr, "Could not load engine\n");
			return false;
		}
	}

	this->parser = new basics::CParser_CommentStore(parser_config);
	if (!this->parser)
	{
		fprintf(stderr, "Error creating parser\n");
		return false;
	}
	return true;
}







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
