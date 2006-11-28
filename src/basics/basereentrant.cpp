#include "basesync.h"
#include "basereentrant.h"

USING_NAMESPACE(basics)

#ifdef WIN32

char* strtok_r( char *strToken, const char *strDelimit, char**pt)
{
	if(strDelimit)
	{
		static char* intern=NULL;
		if( NULL==pt )
			pt  = &intern;

		if(strToken)
			*pt = strToken;

		if(*pt && **pt )
		{	
			char*ret = *pt;
			while( *(*pt) )
			{
				if( strchr(strDelimit, *(*pt)) )
				{
					*(*pt)++='\0';
					break;
				}
				else
					++(*pt);
			}
			return ret;
		}
	}
	return NULL;
}

#endif



const char* strerror(int err, char* buf, size_t size)
{
	static Mutex mx;
	ScopeLock sl(mx);
	if(buf)
	{
		char*p = ::strerror(err);
		if(p)
		{
			if( hstrlen(p)+1 < size ) size = hstrlen(p)+1;
			memcpy(buf,p,size);
			buf[size-1]=0; //force EOS
		}
		else
			*buf=0;
	}
	return buf;
}

