#ifndef __BASEFILE_H__
#define __BASEFILE_H__

#include "basetypes.h"
#include "baseobjects.h"


///////////////////////////////////////////////////////////////////////////////
// seek defines do not always exist
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif


///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define PATHSEP '\\'
#else/////////////////////////
#define PATHSEP '/'
#endif////////////////////////


///////////////////////////////////////////////////////////////////////////////
// basic callback object
///////////////////////////////////////////////////////////////////////////////
class CFileProcessor : public noncopyable
{
public:
	CFileProcessor()			{}
	virtual ~CFileProcessor()	{}
	virtual bool process(const char *name) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
// basic checkings
char* checkPath(char *path, const char*srcpath);
bool findFiles(const char *p, const char *pat, CFileProcessor& fp);
bool isDirectory(const char*name);
bool isFile(const char*name);

///////////////////////////////////////////////////////////////////////////////
// fopen replacement, 
//!! buffer overflow checking
inline FILE* safefopen(const char*name, const char*option)
{	// windows MAXPATH is 260, unix is longer
	char	 namebuf[2048];
	checkPath(namebuf,name);
	return fopen( namebuf, option);
}


//////////////////////////////////////////////////////////////////////////
// file class
// behaves like FILE with OO extension
// supports only signed 32bit addressing, so filesize is 2GB max 
// though it can use 64bit on 64 unix
//////////////////////////////////////////////////////////////////////////
class CFile : public global, public noncopyable
{
	FILE *cFile;
public:
	CFile() : cFile(NULL)
	{}
	CFile(const char* name, const char* mode="rb") : cFile(NULL)
							{ open(name, mode); }
	~CFile()				{ close(); }
	bool ok()				{ return (cFile!=NULL); }

	bool is_open()			{ return (cFile!=NULL); }
	bool clear()			{ return true; }

	bool open(const char* name, const char* mode="rwb")
	{
		if(cFile)	close();
		if(name)	cFile=safefopen(name, mode);
		return is_open();
	}
	bool close()
	{
		if(cFile)
		{
			fclose(cFile); 
			cFile=NULL;
		}
		return true;
	}

	size_t filesize()	
	{
		long end, pos = ftell(cFile);
		fseek(cFile, 0, SEEK_END);
		end = ftell(cFile);
		fseek(cFile, pos, SEEK_SET);
		return end;
	}
	size_t size()			{ return filesize(); }

	bool eof()
	{
		if(cFile)
		{
			long end, pos = ftell(cFile);
			fseek(cFile, 0, SEEK_END);
			end = ftell(cFile);
			fseek(cFile, pos, SEEK_SET);
			return pos<end;
		}
		// if no file we are eof
		return true;
	}

	FILE* operator()()	{ return cFile; }
	operator FILE*()	{ return cFile; }

	ssize_t getline(char *buf, size_t maxlen);
	ssize_t writeline(const char *buf, size_t maxlen=0);
	
	long getpos()			
	{
		if(cFile) 
			return ftell(cFile);
		return 0;
	}
	bool setpos(long pos)
	{
		if(cFile)
			return 0==fseek(cFile,pos, SEEK_SET); 
		return false;
	}
	long tellg()			{ return getpos(); }
	bool seekg(long pos)	{ return setpos(pos); }
};






#endif //__BASEFILE_H__
