// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "utils.h"
#include "showmsg.h"
#include "malloc.h"
#include "mmo.h"

///////////////////////////////////////////////////////////////////////////////
/// dumps an uchar array as hex and ascii.
/// for 64bit systems, the print does not fit in standard console width (80 chars)
void dump(unsigned char *buf, size_t sz)
{
	size_t icnt,jcnt;
	ShowMessage("         Hex                                                  ASCII\n");
	ShowMessage("         -----------------------------------------------      ----------------");
	for(icnt=0; icnt<sz; icnt+=16)
	{
		ShowMessage("\n%p ", &buf[icnt]);
		for (jcnt=icnt; jcnt<icnt+16; ++jcnt)
		{
			if (jcnt < sz)
				ShowMessage("%02hX ", buf[jcnt]);
			else
				ShowMessage("   ");
		}
		ShowMessage("  |  ");
		for (jcnt=icnt;jcnt<icnt+16;++jcnt)
		{
			if (jcnt < sz)
			{
				if(buf[jcnt] > 31 && buf[jcnt] < 127)
					ShowMessage("%c", buf[jcnt]);
				else
					ShowMessage(".");
			}
			else
				ShowMessage(" ");
		}
	}
	ShowMessage("\n");
}

///////////////////////////////////////////////////////////////////////////////
/// test for valid email structure
bool email_check(const char *email)
{
	if( email )
	{
		const char* ip;
		size_t sz = (email)?strlen(email):0;

		if( sz > 3 && sz<40 &&
			// part of RFC limits (official reference of e-mail description)
			email[sz-1] != '@' && email[sz-1] != '.' &&
			(ip=strrchr(email, '@')) != NULL &&
			strstr(ip, "@.") == NULL &&
			strstr(ip, "..") == NULL &&
			strchr(ip, ';') == NULL )
		{
			for(; *ip; ++ip)
			{
				if( *ip>0 && *ip<=32 )
					return false;
			}
			return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// suppress control characters in a string.
bool remove_control_chars(char *str)
{
	bool change = false;
	if(str)
	while( *str )
	{	// replace control chars 
		// but skip chars >0x7F which are negative in char representations
		if ( (*str<32) && (*str>0) )
		{
			*str = '_';
			change = true;
		}
		str++;
	}
	return change;
}
///////////////////////////////////////////////////////////////////////////////
/// create a lowercase copy of a string.
/// without target size checking
const char *strcpytolower(char *tar, const char *src)
{	// without target size checking, actually unused
	if(tar)
	{
		char *ip=tar;
		if(src)
		{
			for(; *src; ++ip, ++src)
				*ip = basics::stringcheck::tolower(*src);
		}
		*ip=0;
		return tar;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// create a lowercase copy of a string.
const char *strcpytolower(char *tar, size_t sz, const char *src)
{
	if(tar)
	{
		char *ip=tar;
		const char*ep=tar+sz-1;
		if(src)
		{
			for(; *src && ip<ep; ++ip, ++src)
				*ip = basics::stringcheck::tolower(*src);
		}
		*ip=0;
		return tar;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// create a copy of a string with sizecheck.
/// systems strncpy doesnt append the trailing 0 if the string is too long
/// so this is done in the wrapper here
const char *safestrcpy(char *tar, size_t sz, const char *src)
{
	if(tar)
	{
		if(src)
		{
			::strncpy(tar,src,sz);
			// systems strncpy doesnt append the trailing 0 if the string is too long.
			tar[sz-1]=0;
		}
		else
			tar[0]=0;
		return tar;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// create a copy of a string and do single char replacements.
const char *replacecpy(char *tar, size_t sz, const char* src, char rplc, char with)
{
	if(tar)
	{	
		char *ip=tar;
		const char *ep=tar+sz-1;
		if(src)
		{
			for(; *src && ip<ep; ++ip, ++src)
				*ip = (*src == rplc) ? with : *src;
		}
		*ip=0;
		return tar;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// copy a mapname to transmission buffer and add the client extension.
const char *mapname2buffer(unsigned char *buf, size_t sz, const char *mapname)
{
	if(buf)
	{
		if(mapname)
		{
			unsigned char*ip = buf;
			const unsigned char *ep= buf+sz-5; // space for ".gat<eos>"
			for(; *mapname && *mapname!='.' && ip<ep; ++ip, ++mapname)
				*ip = basics::stringcheck::tolower(*mapname);
			// append the extension
			*ip++ = '.';
			*ip++ = 'g';
			*ip++ = 'a';
			*ip++ = 't';
			*ip = 0;
		}
		else
			buf[0]=0;
		return (const char *)buf;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// copy a mapname from (transmission) buffer and remove the extension.
const char *buffer2mapname(char *mapname, size_t sz, const char *buf)
{
	if(mapname)
	{
		if(buf)
		{
			char*ip = mapname, *ep= mapname+sz-1;
			while(*buf && *buf!='.' && ip<ep)
				*ip++ = basics::stringcheck::tolower(*buf++);
			*ip = 0;
		}
		else
			mapname[0]=0;
		return mapname;
	}
	return "";
}
///////////////////////////////////////////////////////////////////////////////
/// check if line is empty or a comment line.
const char *is_valid_line(const char *line)
{	// does not process the string
	// skip whitespaces and returns (0x09-0x0D or 0x20) 
	// and return NULL on EOF or following "//"
	if(line)
	{
		while( *line==0x20 || (*line>=0x09 && *line<=0x0D) ) ++line;
		if(*line && (line[0]!='/' || line[1]!='/'))
			return line;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
/// remove whitespaces and comments.
/// return number of chars in remaining line
size_t prepare_line(char *line)
{	// process the string
	// does itrim behaviour internally also breaks on "//"
	// returns remaining number of chars in line
	if(line)
	{	
		char *ip=line, *kp=line, mk=0;
		while(*ip && basics::stringcheck::isspace(*ip) )
			++ip;
		while(*ip)
		{
			if( basics::stringcheck::isspace(*ip) )
				mk=' ', ++ip;
			else if(ip[0]=='/' && ip[1]=='/')
				break;
			else
			{
				if( mk )
					*kp++=mk, mk=0;
				*kp++ = *ip++;
			}
		}
		*kp=0;
		return kp-line;
	}
	return 0;
}

