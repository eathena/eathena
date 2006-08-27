// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "utils.h"
#include "showmsg.h"
#include "malloc.h"
#include "mmo.h"

void dump(unsigned char *buffer, size_t num)
{
	size_t icnt,jcnt;
	ShowMessage("         Hex                                                  ASCII\n");
	ShowMessage("         -----------------------------------------------      ----------------");
	
	for (icnt=0;icnt<num;icnt+=16)
	{
		ShowMessage("\n%p ",&buffer[icnt]);
		for (jcnt=icnt;jcnt<icnt+16;++jcnt)
		{
			if (jcnt < num)
				ShowMessage("%02hX ",buffer[jcnt]);
			else
				ShowMessage("   ");
		}
		ShowMessage("  |  ");
		for (jcnt=icnt;jcnt<icnt+16;++jcnt)
		{
			if (jcnt < num)
			{
				if (buffer[jcnt] > 31 && buffer[jcnt] < 127)
					ShowMessage("%c",buffer[jcnt]);
				else
					ShowMessage(".");
			}
			else
				ShowMessage(" ");
		}
	}
	ShowMessage("\n");
}


int config_switch(const char *str, int min, int max)
{
	char *ss=NULL;
	if (strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja") == 0 || strcasecmp(str, "si") == 0)
		return 1;
	if (strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0)
		return 0;
	long val = strtol(str, &ss, 0);
	return (val>=max)?max:(val<=min)?min:val;
}


//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
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

//------------------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid). by [Yor]
//------------------------------------------------------------
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
			while(*ip)
			{
				if( *ip>0 && *ip<=32 )
					return false;
				ip++;
			}
			return true;
		}
	}
	return false;
}

const char *strcpytolower(char *tar, const char *str)
{
	char *p=tar;
	if(str && p)
	while(*str) 
	{
		*p = basics::stringcheck::tolower(*str);
		p++, str++;
	}
	if(p) *p=0;
	return tar;
}
const char *strcpytolower(char *tar, size_t sz, const char *str)
{
	char *p=tar;
	if(str && p)
	while(*str) 
	{
		*p = basics::stringcheck::tolower(*str);
		p++, str++;
		if(tar+sz-1<=p)
			break;
	}
	if(p) *p=0;
	return tar;
}

const char *safestrcpy(char *tar, size_t cnt, const char *src)
{
	if(tar)
	{
		if(src)
		{
			::strncpy(tar,src,cnt);
			// systems strncpy doesnt append the trailing 0 if the string is too long.
			tar[cnt-1]=0;
		}
		else
			tar[0]=0;
	}
	return tar;
}
const char *replacecpy(char *tar, size_t sz, const char* src, char rplc, char with)
{
	if(tar)
	{	char *strtmp=tar;
		if(src)
		{
			const char*ip=src;
			while( *ip && (src+sz-1 > ip) )
			{
				if(*ip == rplc)
					*strtmp++ =with, ip++;
				else
					*strtmp++ = *ip++;
			}
		}
		*strtmp=0;
	}
	return tar;
}

const char *mapname2buffer(unsigned char *buffer, size_t sz, const char *mapname)
{
	if(buffer)
	{
		if(mapname)
		{
			unsigned char*ip = buffer, *ep= buffer+sz-5; // space for ".gat<eos>"
			while(*mapname && *mapname!='.' && ip<ep)
				*ip++ = basics::stringcheck::tolower(*mapname++);
			*ip++ = '.';
			*ip++ = 'g';
			*ip++ = 'a';
			*ip++ = 't';
			*ip = 0;
		}
		else
			buffer[0]=0;
		return (const char *)buffer;
	}
	return "";
}
const char *buffer2mapname(char *mapname, size_t sz, const char *buffer)
{
	if(mapname)
	{
		if(buffer)
		{
			char*ip = mapname, *ep= mapname+sz-1;
			while(*buffer && *buffer!='.' && ip<ep)
				*ip++ = basics::stringcheck::tolower(*buffer++);
			*ip = 0;
		}
		else
			mapname[0]=0;
		return mapname;
	}
	return "";
}

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








