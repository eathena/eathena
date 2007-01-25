#ifndef _SCRIPTPRN_
#define _SCRIPTPRN_

#include "baseparser.h"
#include "basevariant.h"

///////////////////////////////////////////////////////////////////////////////
char unescape(const char *& str);
void str2id(char*id, size_t sz, const char*str);
void str2name(char*name, size_t sz, const char*str);
void str2strip_quotes(char*target, size_t sz, const char*str);



///////////////////////////////////////////////////////////////////////////////
namespace print
{

struct console_printer : public basics::noncopyable
{
	FILE *logfile;
public:
	FILE *output;	// output, defaults to stdout

	console_printer() : logfile(NULL),output(stdout)
	{}
	console_printer(FILE *outfile) : logfile(NULL),output(outfile?outfile:stdout)
	{}

	virtual ~console_printer()
	{
		if(logfile) fclose(logfile);
	}

	void open_log(const char* name)
	{
		if(logfile) fclose(logfile);
		if(name) logfile = fopen(name, "ab");
	}
	void close_log()
	{
		if(logfile) fclose(logfile);
	}

	template<class T>
	void put(const T& t)
	{
		static basics::string<> str;
		str.assign(t);
		this->put(str.c_str());
	}
	void put(const basics::string<>& str)	{ this->put(str.c_str()); }
	void put(const char *str);
	virtual void put(const char c)
	{	// ignore carriage return
		fputc('c', this->output);
	}
};

inline console_printer& operator <<(console_printer& prn, const char t)					{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const char *t)				{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const int t)					{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const unsigned int t)			{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const long t)					{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const unsigned long t)		{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const int64 t)				{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const uint64 t)				{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const double t)				{ prn.put(t); return prn; }
inline console_printer& operator <<(console_printer& prn, const basics::variant t)		{ prn.put(t.get_string()); return prn; }
inline console_printer& operator <<(console_printer& prn, const basics::string<> t)		{ prn.put(t.c_str()); return prn; }


extern console_printer message;
extern console_printer warning;
extern console_printer error;

}
// end namespace print
///////////////////////////////////////////////////////////////////////////////


struct default_printer : public print::console_printer
{
protected:
	// internal use
	bool newline;	// detects newline, adds scope indentation
public:
	size_t scope;	// scope counter
	bool ignore_nl;	// ignores newlines (ie. for itemscripts)

	default_printer() : newline(false),scope(0),ignore_nl(false)
	{}

	virtual ~default_printer()
	{}

	int log(const char*fmt, ...);
	int log(basics::CParser_CommentStore& parser, int rtpos);

	virtual void put(const char c);


	void print_id(const char* str);
	void print_name(const char* str);
	void print_without_quotes(const char* str);

	void print_comments(basics::CParser_CommentStore& parser, int rtpos);
	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent) { return true; }
};









#endif//_SCRIPTPRN_

