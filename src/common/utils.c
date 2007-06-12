// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>
#endif

#ifdef UTIL_DUMP
void dump(const unsigned char* buffer, int num)
{
	int icnt, jcnt;

	printf("         Hex                                                  ASCII\n");
	printf("         -----------------------------------------------      ----------------");

	for (icnt = 0; icnt < num; icnt += 16)
	{
		printf("\n%p ", &buffer[icnt]);
		for (jcnt = icnt; jcnt < icnt + 16; ++jcnt)
		{
			if (jcnt < num)
				printf("%02hX ", buffer[jcnt]);
			else
				printf("   ");
		}

		printf("  |  ");

		for (jcnt = icnt; jcnt < icnt + 16; ++jcnt)
		{
			if (jcnt < num) {
				if (buffer[jcnt] > 31 && buffer[jcnt] < 127)
					printf("%c", buffer[jcnt]);
				else
					printf(".");
			} else
				printf(" ");
		}
	}
	printf("\n");
}
#endif

/////////////////////////////////////////////////////////////////////
// StringBuf - dynamic string
//
// @author MouseJstr (original)

/// Allocates a StringBuf
struct StringBuf* StringBuf_Malloc() 
{
	struct StringBuf* self;
	CREATE(self, struct StringBuf, 1);
	StringBuf_Init(self);
	return self;
}

/// Initializes a previously allocated StringBuf
void StringBuf_Init(struct StringBuf* self)
{
	self->max_ = 1024;
	self->ptr_ = self->buf_ = (char*)aMallocA(self->max_ + 1);
}

/// Appends the result of printf to the StringBuf
int StringBuf_Printf(struct StringBuf* self, const char* fmt, ...)
{
	int len;
	va_list ap;

	va_start(ap, fmt);
	len = StringBuf_Vprintf(self, fmt, ap);
	va_end(ap);

	return len;
}

/// Appends the result of vprintf to the StringBuf
int StringBuf_Vprintf(struct StringBuf* self, const char* fmt, va_list ap)
{
	int n, size, off;

	for(;;)
	{
		/* Try to print in the allocated space. */
		size = self->max_ - (self->ptr_ - self->buf_);
		n = vsnprintf(self->ptr_, size, fmt, ap);
		/* If that worked, return the length. */
		if( n > -1 && n < size )
		{
			self->ptr_ += n;
			return (int)(self->ptr_ - self->buf_);
		}
		/* Else try again with more space. */
		self->max_ *= 2; // twice the old size
		off = (int)(self->ptr_ - self->buf_);
		self->buf_ = (char*)aRealloc(self->buf_, self->max_ + 1);
		self->ptr_ = self->buf_ + off;
	}
}

/// Appends the contents of another StringBuf to the StringBuf
int StringBuf_Append(struct StringBuf* self, const struct StringBuf* sbuf)
{
	int available = self->max_ - (self->ptr_ - self->buf_);
	int needed = (int)(sbuf->ptr_ - sbuf->buf_);

	if( needed >= available )
	{
		int off = (int)(self->ptr_ - self->buf_);
		self->max_ += needed;
		self->buf_ = (char*)aRealloc(self->buf_, self->max_ + 1);
		self->ptr_ = self->buf_ + off;
	}

	memcpy(self->ptr_, sbuf->buf_, needed);
	self->ptr_ += needed;
	return (int)(self->ptr_ - self->buf_);
}

// Appends str to the StringBuf
int StringBuf_AppendStr(struct StringBuf* self, const char* str) 
{
	int available = self->max_ - (self->ptr_ - self->buf_);
	int needed = (int)strlen(str);

	if( needed >= available )
	{// not enough space, expand the buffer (minimum expansion = 1024)
		int off = (int)(self->ptr_ - self->buf_);
		self->max_ += max(needed, 1024);
		self->buf_ = (char*)aRealloc(self->buf_, self->max_ + 1);
		self->ptr_ = self->buf_ + off;
	}

	memcpy(self->ptr_, str, needed);
	self->ptr_ += needed;
	return (int)(self->ptr_ - self->buf_);
}

// Returns the length of the data in the Stringbuf
int StringBuf_Length(struct StringBuf* self) 
{
	return (int)(self->ptr_ - self->buf_);
}

/// Returns the data in the StringBuf
char* StringBuf_Value(struct StringBuf* self) 
{
	*self->ptr_ = '\0';
	return self->buf_;
}

/// Clears the contents of the StringBuf
void StringBuf_Clear(struct StringBuf* self) 
{
	self->ptr_ = self->buf_;
}

/// Destroys the StringBuf
void StringBuf_Destroy(struct StringBuf* self)
{
	aFree(self->buf_);
	self->ptr_ = self->buf_ = 0;
	self->max_ = 0;
}

// Frees a StringBuf returned by StringBuf_Malloc
void StringBuf_Free(struct StringBuf* self) 
{
	StringBuf_Destroy(self);
	aFree(self);
}

#ifdef WIN32

static char* checkpath(char *path, const char *srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='/') {
			*p++ = '\\';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;
	
	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");
	
	hFind = FindFirstFile(tmppath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if (strcmp(FindFileData.cFileName, "..") == 0)
				continue;

			sprintf(tmppath,"%s%c%s",path,PATHSEP,FindFileData.cFileName);

			if (FindFileData.cFileName && strstr(FindFileData.cFileName, pattern)) {
				func( tmppath );
			}


			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				findfile(tmppath, pat, func);
			}
		}while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}
#else

#define MAX_DIR_PATH 2048

static char* checkpath(char *path, const char*srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='\\') {
			*p++ = '/';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{	
	DIR* dir;					// pointer to the scanned directory.
	struct dirent* entry;		// pointer to one directory entry.
	struct stat dir_stat;       // used by stat().
	char tmppath[MAX_DIR_PATH+1];
	char path[MAX_DIR_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL) strcpy(path,p);

	// open the directory for reading
	dir = opendir( checkpath(path, path) );
	if (!dir) {
		ShowError("Cannot read directory '%s'\n", path);
		return;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while ((entry = readdir(dir))) {
		// skip the "." and ".." entries.
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		sprintf(tmppath,"%s%c%s",path, PATHSEP, entry->d_name);

		// check if the pattern matchs.
		if (entry->d_name && strstr(entry->d_name, pattern)) {
			func( tmppath );
		}
		// check if it is a directory.
		if (stat(tmppath, &dir_stat) == -1) {
			ShowError("stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if (S_ISDIR(dir_stat.st_mode)) {
			// decent recursivly
			findfile(tmppath, pat, func);
		}
	}//end while
}
#endif

uint8 GetByte(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint8)( (val & 0x000000FF)         );
	case 1: return (uint8)( (val & 0x0000FF00) >> 0x08 );
	case 2: return (uint8)( (val & 0x00FF0000) >> 0x10 );
	case 3: return (uint8)( (val & 0xFF000000) >> 0x18 );
	default:
#if defined(DEBUG)
		ShowDebug("GetByte: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}

uint16 GetWord(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint16)( (val & 0x0000FFFF)         );
	case 1: return (uint16)( (val & 0xFFFF0000) >> 0x10 );
	default:
#if defined(DEBUG)
		ShowDebug("GetWord: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}
uint16 MakeWord(uint8 byte0, uint8 byte1)
{
	return byte0 | (byte1 << 0x08);
}

uint32 MakeDWord(uint16 word0, uint16 word1)
{
	return
		( (uint32)(word0        ) )|
		( (uint32)(word1 << 0x10) );
}
