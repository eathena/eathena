#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "base.h"


int config_switch(const char *str);
bool e_mail_check(const char *email);
bool remove_control_chars(char *str);

void dump(unsigned char *buffer, size_t num);

char* checkpath(char *path, const char* src);
void findfile(const char *p, const char *pat, void (func)(const char*) );

extern inline FILE* savefopen(const char*name, const char*option)
{	// windows MAXPATH is 260, unix is longer
	char	 namebuf[2048];
	checkpath(namebuf,name);
	return fopen( namebuf, option);
}


extern inline const char *tolower(char *str)
{
	char *p=str;
	if(p)
	while(*p) *p++ = tolower(*p);
	return str;
}
extern inline const char *strcpytolower(char *tar, const char *str)
{
	char *p=tar;
	if(str && p)
	while(*str) *p++ = tolower(*str++);
	*p=0;
	return tar;
}

extern inline const char *skip_empty_line(const char *line)
{	// skip whitespaces and returns (0x09-0x0D or 0x20) 
	// and return NULL on EOF or following "//"
	if(line)
	{
		while( isspace(*line++) );
		if(*line && line[0]!='/' && line[1]!='/')
			return line;
	}
	return NULL;
}














struct StringBuf {
	char *buf_;
	char *ptr_;
	unsigned int max_;
};

struct StringBuf * StringBuf_Malloc();
void StringBuf_Init(struct StringBuf *);
int StringBuf_Printf(struct StringBuf *,const char *,...);
int StringBuf_Append(struct StringBuf *,const struct StringBuf *);
char * StringBuf_Value(struct StringBuf *);
void StringBuf_Destroy(struct StringBuf *);
void StringBuf_Free(struct StringBuf *);




#endif//COMMON_UTILS_H
