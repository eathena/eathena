#ifndef _SCRIPTPRN_
#define _SCRIPTPRN_

#include "baseparser.h"


///////////////////////////////////////////////////////////////////////////////
char unescape(const char *& str);
void str2id(char*id, size_t sz, const char*str);
void str2name(char*name, size_t sz, const char*str);
void str2strip_quotes(char*target, size_t sz, const char*str);


///////////////////////////////////////////////////////////////////////////////
struct printer : public basics::noncopyable
{
protected:
	// internal use
	bool newline;	// detects newline, adds scope indentation
	FILE *logfile;
public:
	FILE *output;	// output, defaults to stdout
	size_t scope;	// scope counter
	bool ignore_nl;	// ignores newlines (ie. for itemscripts)

	printer() : newline(false),logfile(NULL),output(stdout),scope(0),ignore_nl(false)
	{}

	virtual ~printer()
	{
		if(logfile) fclose(logfile);
	}

	void open_log()
	{
		if(logfile) fclose(logfile);
		logfile = fopen("converter.log", "wb");
	}
	int log(const char*fmt, ...);
	int log(basics::CParser_CommentStore& parser, int rtpos);



	void put(const char c);
	void put(const char *str);
	template<class T> void put(const T& t)
	{
		static basics::string<> str;
		str.assign(t);
		this->put((const char *)str);
	}

	void print_id(const char* str);
	void print_name(const char* str);
	void print_without_quotes(const char* str);

	void print_comments(basics::CParser_CommentStore& parser, int rtpos);
	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent) =0;
};
inline printer& operator <<(printer& prn, const char t)			{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const char *t)		{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const int t)			{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const unsigned int t)	{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const long t)			{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const unsigned long t){ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const int64 t)		{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const uint64 t)		{ prn.put(t); return prn; }
inline printer& operator <<(printer& prn, const double t)		{ prn.put(t); return prn; }

#endif//_SCRIPTPRN_

