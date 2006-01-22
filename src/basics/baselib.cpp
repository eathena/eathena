#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "baselib.h"

bool Czlib::_CZlib::FreeLib()
{
	if( NULL!=czlib )
	{
		::FreeLibrary(czlib);
		czlib=NULL;
		cinflateInit = NULL;
		cinflate     = NULL;
		cinflateEnd  = NULL;
		cdeflateInit = NULL;
		cdeflate     = NULL;
		cdeflateEnd  = NULL;
	}
	return true;
}
bool Czlib::_CZlib::LoadLib(const char* name)
{
	if( NULL!=czlib )
		FreeLib();

	czlib = ::LoadLibrary(name);
	if( NULL != czlib ) {
		cinflateInit = (int (*)(z_streamp, const char *, int))GetProcAddress(czlib,"inflateInit_");
		cinflate     = (int (*)(z_streamp, int))GetProcAddress(czlib,"inflate");
		cinflateEnd  = (int (*)(z_streamp))GetProcAddress(czlib,"inflateEnd");
		cdeflateInit = (int (*)(z_streamp, int, const char *, int))GetProcAddress(czlib,"deflateInit_");
		cdeflate     = (int (*)(z_streamp, int))GetProcAddress(czlib,"deflate");
		cdeflateEnd  = (int (*)(z_streamp))GetProcAddress(czlib,"deflateEnd");
	}
	else
	{	
		printf("Failed loading %s\n",name);
	}
	return (NULL!=czlib);
}
bool Czlib::_CZlib::isOk()
{
	return (
	// library loaded?
	(NULL != czlib) &&
	// do functions exist (deflate)?
	(NULL != cdeflateInit && NULL != cdeflate && NULL != cdeflateEnd) &&
	// do functions exist (inflate)?
	(NULL != cinflateInit && NULL != cinflate && NULL != cinflateEnd)
			);
}

//////////////////////////////////////////////////////////////////////
// access functions
int Czlib::decode(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen)
{
	// check if necessary functions have been exported
	if( NULL==ptr()->cdeflateInit || NULL==ptr()->cdeflate || NULL==ptr()->cdeflateEnd )
		return 0;

	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = ptr()->cinflateInit(&stream,ZLIB_VERSION, sizeof(z_stream));
	if (err != Z_OK) return err;

	err = ptr()->cinflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		ptr()->cinflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = ptr()->cinflateEnd(&stream);
	return err;
}

int Czlib::encode(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen) 
{
	// check if necessary functions have been exported
	if( NULL==ptr()->cinflateInit || NULL==ptr()->cinflate || NULL==ptr()->cinflateEnd )
		return 0;

	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = ptr()->cdeflateInit(&stream,Z_DEFAULT_COMPRESSION,ZLIB_VERSION, sizeof(z_stream));
	if (err != Z_OK) return err;

	err = ptr()->cdeflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		ptr()->cdeflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = ptr()->cdeflateEnd(&stream);
	return err;
}
