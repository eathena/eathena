
#include "strlib.h"
#include "utils.h"
#include "showmsg.h"
#include "malloc.h"
//-----------------------------------------------
// string lib.

// assuming call convention ...(string1)
// string1 buffer has to be large enough, generates run time errors by memory overwrite when failed
char* jstrescape (char* pt) {
	//copy from here
	char *spt,*sp;
	char *p=pt;
	if(NULL==pt) return NULL;

	//copy string to temporary
	CREATE(spt, char, strlen(pt)+1);
	strcpy (spt,pt);

	sp=spt;
	while (*sp) {
		switch (*sp) {
			case '\'':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			case '\\':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			default:
				*p++ = *sp++;
		}
	}
	*p++ = '\0';
	aFree (spt);
	return pt;
}

// assuming call convention ...(string1, string2)
// string1 buffer has to be large enough, generates run time errors by memory overwrite when failed
char* jstrescapecpy (char* pt, const char* sp) {
	//copy from here
	char *p =pt;
	if( (NULL==pt) || (NULL==sp) ) return NULL;
	while(*sp) {
		switch (*sp) {
			case '\'':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			case '\\':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			default:
				*p++ = *sp++;
		}
	}
	*p++ = '\0';
	return pt;
}

// assuming call convention ...(string1, string2, max_chars_to_copy)
// string1 buffer has to be large enough, generates run time errors by memory overwrite when failed
size_t jmemescapecpy (char* pt, const char* spt, int size) {
	//copy from here
	char *p  =pt;
	const char *sp =spt;
	if( (NULL==pt) || (NULL==spt) ) return 0;

	while ( (sp < spt+size) && *sp) {
		switch (*sp) {
			case '\'':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			case '\\':
				*p++ = '\\';
				*p++ = *sp++;
				break;
			default:
				*p++ = *sp++;
		}
	}
	// copy size is 0 ~ (j-1)
	// and what about the final EOS? You __NEED__ it... 
	*p = '\0';
	return (p-pt);
}

char* trim(char* str)
{
	if(str)
	{
//		size_t pos=0, len=strlen(str);
//		// right trim
//		while( len && isspace( (int)((unsigned char)str[len-1]) ) ) len--;
//		str[len] = 0;
//		// left trim
//		while( pos<len && isspace( (int)((unsigned char)str[pos]) ) ) pos++;
//		if(pos) memmove(str, str+pos, (len-pos+1)*sizeof(char));

		char *src=str, *tar=str, *mk=NULL;
		while(*src && isspace( (int)((unsigned char)*src)) )
			src++;
		while(*src)
		{
			mk = ( isspace( (int)((unsigned char)*src)) )?mk?mk:tar:NULL;
			*tar++ = *src++;
		}
		if(mk) 
			*mk=0;
		else
			*tar=0;
	}
	return str;
}
char* checktrim(char* str, bool removeall)
{
	if(str)
	{
		char *src=str, *tar=str, mk=0;
		while(*src && isspace( (int)((unsigned char)*src)) )
			src++;
		while(*src)
		{
			if( isspace( (int)((unsigned char)*src)) )
				mk=' ', src++;
			else
			{
				if( mk && !removeall )
					*tar++=mk, mk=0;
				*tar++ = *src++;
			}
		}
		*tar=0;
	}
	return str;
}
