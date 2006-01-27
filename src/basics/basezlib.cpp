#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "baselib.h"
#include "basezlib.h"

///////////////////////////////////////////////////////////////////////////////
// direct copy of the minizip environment 
// from zlib v1.2.3 package "zlib/contrib/minizip"
// cleaned for usage inside this environment
//
//!! TODO: remove usused stuff
//!! TODO: integrate to zlib class to enable dynamic zlib loading






///////////////////////////////////////////////////////////////////////////////
// ioapi.h -- IO base function header for compress/uncompress .zip
// files using zlib + zip or unzip API
//
// Version 1.01e, February 12th, 2005
//
// Copyright (C) 1998-2005 Gilles Vollant
#define ZLIB_FILEFUNC_SEEK_CUR (1)
#define ZLIB_FILEFUNC_SEEK_END (2)
#define ZLIB_FILEFUNC_SEEK_SET (0)

#define ZLIB_FILEFUNC_MODE_READ      (1)
#define ZLIB_FILEFUNC_MODE_WRITE     (2)
#define ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#define ZLIB_FILEFUNC_MODE_EXISTING (4)
#define ZLIB_FILEFUNC_MODE_CREATE   (8)



typedef voidpf (*open_file_func) (voidpf opaque, const char* filename, int mode);
typedef uLong  (*read_file_func) (voidpf opaque, voidpf stream, void* buf, uLong size);
typedef uLong  (*write_file_func) (voidpf opaque, voidpf stream, const void* buf, uLong size);
typedef long   (*tell_file_func) (voidpf opaque, voidpf stream);
typedef long   (*seek_file_func) (voidpf opaque, voidpf stream, uLong offset, int origin);
typedef int    (*close_file_func) (voidpf opaque, voidpf stream);
typedef int    (*testerror_file_func) (voidpf opaque, voidpf stream);

typedef struct zlib_filefunc_def_s
{
    open_file_func      zopen_file;
    read_file_func      zread_file;
    write_file_func     zwrite_file;
    tell_file_func      ztell_file;
    seek_file_func      zseek_file;
    close_file_func     zclose_file;
    testerror_file_func zerror_file;
    voidpf              opaque;
} zlib_filefunc_def;

void fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def);

#define ZREAD(filefunc,filestream,buf,size) ((*((filefunc).zread_file))((filefunc).opaque,filestream,buf,size))
#define ZWRITE(filefunc,filestream,buf,size) ((*((filefunc).zwrite_file))((filefunc).opaque,filestream,buf,size))
#define ZTELL(filefunc,filestream) ((*((filefunc).ztell_file))((filefunc).opaque,filestream))
#define ZSEEK(filefunc,filestream,pos,mode) ((*((filefunc).zseek_file))((filefunc).opaque,filestream,pos,mode))
#define ZCLOSE(filefunc,filestream) ((*((filefunc).zclose_file))((filefunc).opaque,filestream))
#define ZERROR(filefunc,filestream) ((*((filefunc).zerror_file))((filefunc).opaque,filestream))





voidpf fopen_file_func (voidpf opaque,
						const char* filename,
						int mode)
{
    FILE* file = NULL;
    const char* mode_fopen = NULL;
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    if ((filename!=NULL) && (mode_fopen != NULL))
        file = fopen(filename, mode_fopen);
    return file;
}

uLong fread_file_func (voidpf opaque,
					   voidpf stream,
					   void* buf,
					   uLong size)
{
    uLong ret;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    return ret;
}

uLong fwrite_file_func (voidpf opaque,
						voidpf stream,
						const void* buf,
						uLong size)
{
    uLong ret;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    return ret;
}


long fseek_file_func (voidpf opaque,
					  voidpf stream,
					  uLong offset,
					  int origin)
{
    int fseek_origin=0;
    long ret;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;
    default: return -1;
    }
    ret = 0;
    fseek((FILE *)stream, offset, fseek_origin);
    return ret;
}

long ftell_file_func (voidpf opaque,
					  voidpf stream)
{
    long ret;
    ret = ftell((FILE *)stream);
    return ret;
}

int fclose_file_func (voidpf opaque,
					  voidpf stream)
{
    int ret;
    ret = fclose((FILE *)stream);
    return ret;
}

int ferror_file_func (voidpf opaque,
					  voidpf stream)
{
    int ret;
    ret = ferror((FILE *)stream);
    return ret;
}

void fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}








///////////////////////////////////////////////////////////////////////////////
// unzip.h -- IO for uncompress .zip files using zlib
// Version 1.01e, February 12th, 2005
//
// Copyright (C) 1998-2005 Gilles Vollant
//
// This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
//   WinZip, InfoZip tools and compatible.
//
// Multi volume ZipFile (span) are not supported.
// Encryption compatible with pkzip 2.04g only supported
// Old compressions used by old PKZip 1.x are not supported
//
//
// I WAIT FEEDBACK at mail info@winimage.com
// Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution
//
// Condition of use and distribution are the same than zlib :
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//
// for more info about .ZIP format, see
//    http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
//    http://www.info-zip.org/pub/infozip/doc/
// PkWare has also a specification at :
//    ftp://ftp.pkware.com/probdesc.zip


///////////////////////////////////////////////////////////////////////////////
// 
#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
// like the STRICT of WIN32, we define a pointer 
// that cannot be converted from (void*) without cast
typedef struct TagunzFile__ { int unused; } unzFile__;
typedef unzFile__ *unzFile;
#else
typedef voidp unzFile;
#endif

///////////////////////////////////////////////////////////////////////////////
// error codes
#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

///////////////////////////////////////////////////////////////////////////////
// tm_unz contain date/time info 
typedef struct tm_unz_s
{
    uInt tm_sec;            // seconds after the minute - [0,59] 
    uInt tm_min;            // minutes after the hour - [0,59] 
    uInt tm_hour;           // hours since midnight - [0,23] 
    uInt tm_mday;           // day of the month - [1,31] 
    uInt tm_mon;            // months since January - [0,11] 
    uInt tm_year;           // years - [1980..2044] 
} tm_unz;

///////////////////////////////////////////////////////////////////////////////
// unz_global_info structure contain global data about the ZIPfile
// These data comes from the end of central dir
typedef struct unz_global_info_s
{
    uLong number_entry;     // total number of entries in the central dir on this disk 
    uLong size_comment;     // size of the global comment of the zipfile 
} unz_global_info;

///////////////////////////////////////////////////////////////////////////////
// unz_file_info contain information about a file in the zipfile
typedef struct unz_file_info_s
{
    uLong version;              // version made by                 2 bytes 
    uLong version_needed;       // version needed to extract       2 bytes 
    uLong flag;                 // general purpose bit flag        2 bytes 
    uLong compression_method;   // compression method              2 bytes 
    uLong dosDate;              // last mod file date in Dos fmt   4 bytes 
    uLong crc;                  // crc-32                          4 bytes 
    uLong compressed_size;      // compressed size                 4 bytes 
    uLong uncompressed_size;    // uncompressed size               4 bytes 
    uLong size_filename;        // filename length                 2 bytes 
    uLong size_file_extra;      // extra field length              2 bytes 
    uLong size_file_comment;    // file comment length             2 bytes 
    uLong disk_num_start;       // disk number start               2 bytes 
    uLong internal_fa;          // internal file attributes        2 bytes 
    uLong external_fa;          // external file attributes        4 bytes 
    tm_unz tmu_date;
} unz_file_info;


///////////////////////////////////////////////////////////////////////////////
// Compare two filename (fileName1,fileName2).
// If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
// If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
//                              or strcasecmp)
// If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
//  (like 1 on Unix, 2 on Windows)
int unzStringFileNameCompare (const char* fileName1,
							  const char* fileName2,
							  int iCaseSensitivity);

///////////////////////////////////////////////////////////////////////////////
// Open a Zip file. path contain the full pathname (by example,
//   on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
//   "zlib/zlib113.zip".
//   If the zipfile cannot be opened (file don't exist or in not valid), the
//     return value is NULL.
//   Else, the return value is a unzFile Handle, usable with other function
//     of this unzip package.
unzFile unzOpen (const char *path);

///////////////////////////////////////////////////////////////////////////////
// Open a Zip file, like unzOpen, but provide a set of file low level API
//    for read/write the zip file (see ioapi.h)
unzFile unzOpen2 (const char *path,
				  zlib_filefunc_def* pzlib_filefunc_def);

///////////////////////////////////////////////////////////////////////////////
// Close a ZipFile opened with unzipOpen.
// If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
//  these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
//  return UNZ_OK if there is no problem. */
int unzClose (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Write info about the ZipFile in the *pglobal_info structure.
// No preparation of the structure is needed
// return UNZ_OK if there is no problem.
int unzGetGlobalInfo (unzFile file,
					  unz_global_info *pglobal_info);


///////////////////////////////////////////////////////////////////////////////
// Get the global comment string of the ZipFile, in the szComment buffer.
// uSizeBuf is the size of the szComment buffer.
// return the number of byte copied or an error code <0
int unzGetGlobalComment (unzFile file,
						 char *szComment,
						 uLong uSizeBuf);



///////////////////////////////////////////////////////////////////////////////
// Unzip package allow you browse the directory of the zipfile
// Set the current file of the zipfile to the first file.
// return UNZ_OK if there is no problem
int unzGoToFirstFile (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Set the current file of the zipfile to the next file.
// return UNZ_OK if there is no problem
// return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
int unzGoToNextFile (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Try locate the file szFileName in the zipfile.
// For the iCaseSensitivity signification, see unzStringFileNameCompare
// return value :
// UNZ_OK if the file is found. It becomes the current file.
// UNZ_END_OF_LIST_OF_FILE if the file is not found
int unzLocateFile (unzFile file,
				   const char *szFileName,
				   int iCaseSensitivity);


///////////////////////////////////////////////////////////////////////////////
// Ryan supplied functions
// unz_file_info contain information about a file in the zipfile
typedef struct unz_file_pos_s
{
    uLong pos_in_zip_directory;   // offset in zip file directory
    uLong num_of_file;            // # of file
} unz_file_pos;

///////////////////////////////////////////////////////////////////////////////
//
int unzGetFilePos(unzFile file,
				  unz_file_pos* file_pos);

///////////////////////////////////////////////////////////////////////////////
//
int unzGoToFilePos(unzFile file,
				   unz_file_pos* file_pos);

///////////////////////////////////////////////////////////////////////////////
// Get Info about the current file
// if pfile_info!=NULL, the *pfile_info structure will contain somes info 
// about the current file
// if szFileName!=NULL, the filename string will be copied in szFileName
// (fileNameBufferSize is the size of the buffer)
// if extraField!=NULL, the extra field information will be copied in extraField
// (extraFieldBufferSize is the size of the buffer).
// This is the Central-header version of the extra field
// if szComment!=NULL, the comment string of the file will be copied 
// in szComment (commentBufferSize is the size of the buffer)
int unzGetCurrentFileInfo (unzFile file,
						   unz_file_info *pfile_info,
						   char *szFileName,
						   uLong fileNameBufferSize,
						   void *extraField,
						   uLong extraFieldBufferSize,
						   char *szComment,
						   uLong commentBufferSize);

///////////////////////////////////////////////////////////////////////////////
// for reading the content of the current zipfile, you can open it, read data
// from it, and close it (you can close it before reading all the file)

///////////////////////////////////////////////////////////////////////////////
// Open for reading data the current file in the zipfile.
// If there is no error, the return value is UNZ_OK.
int unzOpenCurrentFile (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Open for reading data the current file in the zipfile.
// password is a crypting password
// If there is no error, the return value is UNZ_OK.
int unzOpenCurrentFilePassword (unzFile file,
								const char* password);

///////////////////////////////////////////////////////////////////////////////
// Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
// if raw==1, *method will receive method of compression, 
// *level will receive level of compression
// note : you can set level parameter as NULL (if you did not want known level,
// but you CANNOT set method parameter as NULL
int unzOpenCurrentFile2 (unzFile file,
						 int* method,
						 int* level,
						 int raw);

///////////////////////////////////////////////////////////////////////////////
// Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
// if raw==1, *method will receive method of compression, 
// *level will receive level of compression
// note : you can set level parameter as NULL (if you did not want known level,
// but you CANNOT set method parameter as NULL
int unzOpenCurrentFile3 (unzFile file,
						 int* method,
						 int* level,
						 int raw,
						 const char* password);

///////////////////////////////////////////////////////////////////////////////
// Close the file in zip opened with unzOpenCurrentFile
// Return UNZ_CRCERROR if all the file was read but the CRC is not good
int unzCloseCurrentFile (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Read bytes from the current file (opened by unzOpenCurrentFile)
// buf contain buffer where data must be copied
// len the size of buf.
//  return the number of byte copied if somes bytes are copied
//  return 0 if the end of file was reached
//  return <0 with error code if there is an error
//  (UNZ_ERRNO for IO error, or zLib error for uncompress error)
int unzReadCurrentFile (unzFile file,
						voidp buf,
						unsigned len);

///////////////////////////////////////////////////////////////////////////////
// Give the current position in uncompressed data
z_off_t unztell (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// return 1 if the end of file was reached, 0 elsewhere
int unzeof (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Read extra field from the current file (opened by unzOpenCurrentFile)
// This is the local-header version of the extra field (sometimes, there is
// more info in the local-header version than in the central-header)
// if buf==NULL, it return the size of the local extra field
// if buf!=NULL, len is the size of the buffer, the extra header is copied in buf.
// the return value is the number of bytes copied in buf, or (if <0)
// the error code
int unzGetLocalExtrafield (unzFile file,
						   voidp buf,
						   unsigned len);

///////////////////////////////////////////////////////////////////////////////
// Get the current file offset
uLong unzGetOffset (unzFile file);

///////////////////////////////////////////////////////////////////////////////
// Set the current file offset
int unzSetOffset (unzFile file, uLong pos);








///////////////////////////////////////////////////////////////////////////////
// class implementation
#ifdef BASE_ZLIB_DYNAMIC

bool CZlib::_CZlib::FreeLib()
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

bool CZlib::_CZlib::LoadLib(const char* name)
{
	if( NULL!=czlib )
		FreeLib();

	czlib = ::LoadLibrary(name);
	if( NULL != czlib )
	{
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

bool CZlib::_CZlib::isOk()
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

#endif

//////////////////////////////////////////////////////////////////////
// access functions
int CZlib::decode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen)
{
#ifdef BASE_ZLIB_DYNAMIC
	// check if necessary functions have been exported
	if( NULL==ptr()->cdeflateInit || NULL==ptr()->cdeflate || NULL==ptr()->cdeflateEnd )
		return 0;
#endif
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((uLong)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cinflateInit(&stream,ZLIB_VERSION, sizeof(z_stream));
#else
	err = ::inflateInit(&stream);
#endif

	if (err != Z_OK) return err;

#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cinflate(&stream, Z_FINISH);
#else
	err = ::inflate(&stream, Z_FINISH);
#endif

	if (err != Z_STREAM_END)
	{
#ifdef BASE_ZLIB_DYNAMIC
		ptr()->cinflateEnd(&stream);
#else
		::inflateEnd(&stream);
#endif
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;

	
#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cinflateEnd(&stream);
#else
	err = ::inflateEnd(&stream);
#endif

	return err;
}

int CZlib::encode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen) 
{
#ifdef BASE_ZLIB_DYNAMIC
	// check if necessary functions have been exported
	if( NULL==ptr()->cinflateInit || NULL==ptr()->cinflate || NULL==ptr()->cinflateEnd )
		return 0;
#endif

	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((uLong)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cdeflateInit(&stream,Z_BEST_COMPRESSION,ZLIB_VERSION, sizeof(z_stream));
#else
	err = ::deflateInit(&stream, Z_BEST_COMPRESSION);
#endif
	if (err != Z_OK) return err;
#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cdeflate(&stream, Z_FINISH);
#else
	err = ::deflate(&stream, Z_FINISH);
#endif
	if (err != Z_STREAM_END)
	{
#ifdef BASE_ZLIB_DYNAMIC
		ptr()->cdeflateEnd(&stream);
#else
		::deflateEnd(&stream);
#endif
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;
#ifdef BASE_ZLIB_DYNAMIC
	err = ptr()->cdeflateEnd(&stream);
#else
	err = ::deflateEnd(&stream);
#endif

	return err;
}



bool CZlib::deflate(unsigned char *dest, unsigned long& destLen, const char *source, const char *filename, const char *pass)
{
	unz_file_info pfile_info;
	int err = UNZ_OK;
	unzFile uf = unzOpen(source);
	
	if (uf == NULL)
	{
		printf("Cannot open %s\n", source);
		err = UNZ_ERRNO;
	}
	else if( (err=unzLocateFile(uf, filename, 0)) != UNZ_OK)
	{
		printf("file %s not found in the zipfile\n", filename);
	}
	else if( (err=unzOpenCurrentFilePassword(uf, pass)) != UNZ_OK)
	{
		printf("cannot open zipped file %s\n", filename);
	}
	else if( (err=unzGetCurrentFileInfo(uf, &pfile_info, NULL,0,NULL,0,NULL,0)) != UNZ_OK)
	{
		printf("cannot get info of zipped file %s\n", filename);
	}
	else if(dest==NULL)
	{	// just asked for deflated size
		destLen = pfile_info.uncompressed_size;
	}
	else
	{
		if(destLen>pfile_info.uncompressed_size)
			destLen = pfile_info.uncompressed_size;
		err = unzReadCurrentFile(uf, dest, destLen);
	}
	if(uf)
	{
		err = unzCloseCurrentFile(uf);
		if (err != UNZ_OK)
			printf("error %d with zipfile in unzCloseCurrentFile\n", err);
	}
	return (err == UNZ_OK);
}






























/* crypt.h -- base code for crypt/uncrypt ZIPfile


   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   This code is a modified version of crypting code in Infozip distribution

   The encryption/decryption parts of this source code (as opposed to the
   non-echoing password parts) were originally written in Europe.  The
   whole source package can be freely distributed, including from the USA.
   (Prior to January 2000, re-export from the US was a violation of US law.)

   This encryption code is a direct transcription of the algorithm from
   Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
   file (appnote.txt) is distributed with the PKZIP program (even in the
   version without encryption capabilities).

   If you don't need crypting in your application, just define symbols
   NOCRYPT and NOUNCRYPT.

   This code support the "Traditional PKWARE Encryption".

   The new AES encryption added on Zip format by Winzip (see the page
   http://www.winzip.com/aes_info.htm ) and PKWare PKZip 5.x Strong
   Encryption is not supported.
*/

#define CRC32(c, b) ((*(pcrc_32_tab+(((int)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
static int decrypt_byte(unsigned long* pkeys, const unsigned long* pcrc_32_tab)
{
    unsigned temp;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    temp = ((unsigned)(*(pkeys+2)) & 0xffff) | 2;
    return (int)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
static int update_keys(unsigned long* pkeys,const unsigned long* pcrc_32_tab,int c)
{
    (*(pkeys+0)) = CRC32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      register int keyshift = (int)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = CRC32((*(pkeys+2)), keyshift);
    }
    return c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
static void init_keys(const char* passwd,unsigned long* pkeys,const unsigned long* pcrc_32_tab)
{
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    while (*passwd != '\0') {
        update_keys(pkeys,pcrc_32_tab,(int)*passwd);
        passwd++;
    }
}

#define zdecode(pkeys,pcrc_32_tab,c) \
    (update_keys(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#define zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_keys(pkeys,pcrc_32_tab,c), t^(c))

#ifdef INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#define RAND_HEAD_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL     /* use PI as default pattern */
#  endif

static int crypthead(passwd, buf, bufSize, pkeys, pcrc_32_tab, crcForCrypting)
    const char *passwd;         /* password string */
    unsigned char *buf;         /* where to write header */
    int bufSize;
    unsigned long* pkeys;
    const unsigned long* pcrc_32_tab;
    unsigned long crcForCrypting;
{
    int n;                       /* index in random header */
    int t;                       /* temporary */
    int c;                       /* random byte */
    unsigned char header[RAND_HEAD_LEN-2]; /* random header */
    static unsigned calls = 0;   /* ensure different random header each time */

    if (bufSize<RAND_HEAD_LEN)
      return 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    if (++calls == 1)
    {
        srand((unsigned)(time(NULL) ^ ZCR_SEED2));
    }
    init_keys(passwd, pkeys, pcrc_32_tab);
    for (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        header[n] = (unsigned char)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_keys(passwd, pkeys, pcrc_32_tab);
    for (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        buf[n] = (unsigned char)zencode(pkeys, pcrc_32_tab, header[n], t);
    }
    buf[n++] = zencode(pkeys, pcrc_32_tab, (int)(crcForCrypting >> 16) & 0xff, t);
    buf[n++] = zencode(pkeys, pcrc_32_tab, (int)(crcForCrypting >> 24) & 0xff, t);
    return n;
}

#endif






/* unzip.c -- IO for uncompress .zip files using zlib
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   Read unzip.h for more info
*/

/* Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced in terms of
compatibility with older software. The following is from the original crypt.c. Code
woven in by Terry Thorsen 1/2003.
*/
/*
  Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
  crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

  The encryption/decryption parts of this source code (as opposed to the
  non-echoing password parts) were originally written in Europe.  The
  whole source package can be freely distributed, including from the USA.
  (Prior to January 2000, re-export from the US was a violation of US law.)
 */

/*
  This encryption code is a direct transcription of the algorithm from
  Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
  file (appnote.txt) is distributed with the PKZIP program (even in the
  version without encryption capabilities).
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif


#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */


#ifndef CASESENSITIVITYDEFAULT_NO
#  if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)




const char unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - http://www.winimage.com/zLibDll";

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct unz_file_info_internal_s
{
    uLong offset_curfile;/* relative offset of local header 4 bytes */
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
typedef struct
{
    char  *read_buffer;         /* internal buffer for compressed data */
    z_stream stream;            /* zLib stream structure for inflate */

    uLong pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong stream_initialised;   /* flag set if stream structure is initialised*/

    uLong offset_local_extrafield;/* offset of the local extra field */
    uInt  size_local_extrafield;/* size of the local extra field */
    uLong pos_local_extrafield;   /* position in the local extra field in read*/

    uLong crc32;                /* crc32 of all data uncompressed */
    uLong crc32_wait;           /* crc32 we must obtain after decompress all */
    uLong rest_read_compressed; /* number of byte to be decompressed */
    uLong rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc_def z_filefunc;
    voidpf filestream;        /* io structore of the zipfile */
    uLong compression_method;   /* compression method (0==store) */
    uLong byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    int   raw;
} file_in_zip_read_info_s;


/* unz_s contain internal information about the zipfile
*/
typedef struct
{
    zlib_filefunc_def z_filefunc;
    voidpf filestream;        /* io structore of the zipfile */
    unz_global_info gi;       /* public global information */
    uLong byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    uLong num_file;             /* number of the current file in the zipfile*/
    uLong pos_in_central_dir;   /* pos of the current file in the central dir*/
    uLong current_file_ok;      /* flag about the usability of the current file*/
    uLong central_pos;          /* position of the beginning of the central dir*/

    uLong size_central_dir;     /* size of the central directory  */
    uLong offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info cur_file_info; /* public info about the current file in zip*/
    unz_file_info_internal cur_file_info_internal; /* private info about it*/
    file_in_zip_read_info_s* pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
    int encrypted;
#    ifndef NOUNCRYPT
    unsigned long keys[3];     /* keys defining the pseudo-random sequence */
    const unsigned long* pcrc_32_tab;
#    endif
} unz_s;



/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/


local int unzlocal_getByte(const zlib_filefunc_def* pzlib_filefunc_def,
						   voidpf filestream,
						   int *pi)
{
    unsigned char c;
    int err = (int)ZREAD(*pzlib_filefunc_def,filestream,&c,1);
    if (err==1)
    {
        *pi = (int)c;
        return UNZ_OK;
    }
    else
    {
        if (ZERROR(*pzlib_filefunc_def,filestream))
            return UNZ_ERRNO;
        else
            return UNZ_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local int unzlocal_getShort(
    const zlib_filefunc_def* pzlib_filefunc_def,
    voidpf filestream,
    uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

local int unzlocal_getLong(
    const zlib_filefunc_def* pzlib_filefunc_def,
    voidpf filestream,
    uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}


/* My own strcmpi / strcasecmp */
local int strcmpcasenosensitive_internal (const char* fileName1,
										  const char* fileName2)
{
    for (;;)
    {
        char c1=*(fileName1++);
        char c2=*(fileName2++);
        if ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        if ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        if (c1=='\0')
            return ((c2=='\0') ? 0 : -1);
        if (c2=='\0')
            return 1;
        if (c1<c2)
            return -1;
        if (c1>c2)
            return 1;
    }
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#else
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
#define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal
#endif

/*
   Compare two filename (fileName1,fileName2).
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
                                                                or strcasecmp)
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
        (like 1 on Unix, 2 on Windows)

*/
int unzStringFileNameCompare (const char* fileName1,
							  const char* fileName2,
							  int iCaseSensitivity)
{
    if (iCaseSensitivity==0)
        iCaseSensitivity=CASESENSITIVITYDEFAULTVALUE;

    if (iCaseSensitivity==1)
        return strcmp(fileName1,fileName2);

    return STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local uLong unzlocal_SearchCentralDir(
    const zlib_filefunc_def* pzlib_filefunc_def,
    voidpf filestream)
{
    unsigned char* buf;
    uLong uSizeFile;
    uLong uBackRead;
    uLong uMaxBack=0xffff; /* maximum size of global comment */
    uLong uPosFound=0;

    if (ZSEEK(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        return 0;


    uSizeFile = ZTELL(*pzlib_filefunc_def,filestream);

    if (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
    if (buf==NULL)
        return 0;

    uBackRead = 4;
    while (uBackRead<uMaxBack)
    {
        uLong uReadSize,uReadPos ;
        int i;
        if (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);
        if (ZSEEK(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            break;

        if (ZREAD(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
            break;

        for (i=(int)uReadSize-3; (i--)>0;)
            if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+i;
                break;
            }

        if (uPosFound!=0)
            break;
    }
    TRYFREE(buf);
    return uPosFound;
}

/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
     "zlib/zlib114.zip".
     If the zipfile cannot be opened (file doesn't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/
unzFile unzOpen2 (const char *path, zlib_filefunc_def* pzlib_filefunc_def)
{
    unz_s us;
    unz_s *s;
    uLong central_pos,uL;

    uLong number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    uLong number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    int err=UNZ_OK;

    if (unz_copyright[0]!=' ')
        return NULL;

    if (pzlib_filefunc_def==NULL)
        fill_fopen_filefunc(&us.z_filefunc);
    else
        us.z_filefunc = *pzlib_filefunc_def;

    us.filestream= (*(us.z_filefunc.zopen_file))(us.z_filefunc.opaque,
                                                 path,
                                                 ZLIB_FILEFUNC_MODE_READ |
                                                 ZLIB_FILEFUNC_MODE_EXISTING);
    if (us.filestream==NULL)
        return NULL;

    central_pos = unzlocal_SearchCentralDir(&us.z_filefunc,us.filestream);
    if (central_pos==0)
        err=UNZ_ERRNO;

    if (ZSEEK(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;

    /* the signature, already checked */
    if (unzlocal_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of this disk */
    if (unzlocal_getShort(&us.z_filefunc, us.filestream,&number_disk)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of the disk with the start of the central directory */
    if (unzlocal_getShort(&us.z_filefunc, us.filestream,&number_disk_with_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir on this disk */
    if (unzlocal_getShort(&us.z_filefunc, us.filestream,&us.gi.number_entry)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir */
    if (unzlocal_getShort(&us.z_filefunc, us.filestream,&number_entry_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((number_entry_CD!=us.gi.number_entry) ||
        (number_disk_with_CD!=0) ||
        (number_disk!=0))
        err=UNZ_BADZIPFILE;

    /* size of the central directory */
    if (unzlocal_getLong(&us.z_filefunc, us.filestream,&us.size_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* offset of start of central directory with respect to the
          starting disk number */
    if (unzlocal_getLong(&us.z_filefunc, us.filestream,&us.offset_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* zipfile comment length */
    if (unzlocal_getShort(&us.z_filefunc, us.filestream,&us.gi.size_comment)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    if (err!=UNZ_OK)
    {
        ZCLOSE(us.z_filefunc, us.filestream);
        return NULL;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;
    us.encrypted = 0;


    s=(unz_s*)ALLOC(sizeof(unz_s));
    *s=us;
    unzGoToFirstFile((unzFile)s);
    return (unzFile)s;
}


unzFile unzOpen (const char *path)
{
    return unzOpen2(path, NULL);
}

/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
int unzClose (unzFile file)
{
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    if (s->pfile_in_zip_read!=NULL)
        unzCloseCurrentFile(file);

    ZCLOSE(s->z_filefunc, s->filestream);
    TRYFREE(s);
    return UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
int unzGetGlobalInfo (unzFile file,unz_global_info *pglobal_info)
{
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    *pglobal_info=s->gi;
    return UNZ_OK;
}


/*
   Translate date/time from Dos format to tm_unz (readable more easilty)
*/
local void unzlocal_DosDateToTmuDate (uLong ulDosDate, tm_unz* ptm)
{
    uLong uDate;
    uDate = (uLong)(ulDosDate>>16);
    ptm->tm_mday = (uInt)(uDate&0x1f) ;
    ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local int unzlocal_GetCurrentFileInfoInternal(unzFile file,
                                                  unz_file_info *pfile_info,
                                                  unz_file_info_internal
                                                  *pfile_info_internal,
                                                  char *szFileName,
                                                  uLong fileNameBufferSize,
                                                  void *extraField,
                                                  uLong extraFieldBufferSize,
                                                  char *szComment,
                                                  uLong commentBufferSize)
{
    unz_s* s;
    unz_file_info file_info;
    unz_file_info_internal file_info_internal;
    int err=UNZ_OK;
    uLong uMagic;
    long lSeek=0;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    if (ZSEEK(s->z_filefunc, s->filestream,
              s->pos_in_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    if (err==UNZ_OK)
        if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        else if (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.version) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.version_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.compression_method) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unzlocal_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info.compressed_size) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info.uncompressed_size) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.size_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.disk_num_start) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&file_info.internal_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
        err=UNZ_ERRNO;

    lSeek+=file_info.size_filename;
    if ((err==UNZ_OK) && (szFileName!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_filename<fileNameBufferSize)
        {
            *(szFileName+file_info.size_filename)='\0';
            uSizeRead = file_info.size_filename;
        }
        else
            uSizeRead = fileNameBufferSize;

        if ((file_info.size_filename>0) && (fileNameBufferSize>0))
            if (ZREAD(s->z_filefunc, s->filestream,szFileName,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek -= uSizeRead;
    }


    if ((err==UNZ_OK) && (extraField!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_file_extra<extraFieldBufferSize)
            uSizeRead = file_info.size_file_extra;
        else
            uSizeRead = extraFieldBufferSize;

        if (lSeek!=0)
            if (ZSEEK(s->z_filefunc, s->filestream,lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            else
                err=UNZ_ERRNO;
        if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
            if (ZREAD(s->z_filefunc, s->filestream,extraField,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek += file_info.size_file_extra - uSizeRead;
    }
    else
        lSeek+=file_info.size_file_extra;


    if ((err==UNZ_OK) && (szComment!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_file_comment<commentBufferSize)
        {
            *(szComment+file_info.size_file_comment)='\0';
            uSizeRead = file_info.size_file_comment;
        }
        else
            uSizeRead = commentBufferSize;

        if (lSeek!=0)
            if (ZSEEK(s->z_filefunc, s->filestream,lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            else
                err=UNZ_ERRNO;
        if ((file_info.size_file_comment>0) && (commentBufferSize>0))
            if (ZREAD(s->z_filefunc, s->filestream,szComment,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek+=file_info.size_file_comment - uSizeRead;
    }
    else
        lSeek+=file_info.size_file_comment;

    if ((err==UNZ_OK) && (pfile_info!=NULL))
        *pfile_info=file_info;

    if ((err==UNZ_OK) && (pfile_info_internal!=NULL))
        *pfile_info_internal=file_info_internal;

    return err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
int unzGetCurrentFileInfo (unzFile file,
                                          unz_file_info *pfile_info,
                                          char *szFileName,
										  uLong fileNameBufferSize,
                                          void *extraField,
										  uLong extraFieldBufferSize,
                                          char *szComment,
										  uLong commentBufferSize)
{
    return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,
                                                szFileName,fileNameBufferSize,
                                                extraField,extraFieldBufferSize,
                                                szComment,commentBufferSize);
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
int unzGoToFirstFile (unzFile file)
{
    int err=UNZ_OK;
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    s->pos_in_central_dir=s->offset_central_dir;
    s->num_file=0;
    err=unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                             &s->cur_file_info_internal,
                                             NULL,0,NULL,0,NULL,0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
int unzGoToNextFile (unzFile file)
{
    unz_s* s;
    int err;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;
    if (s->gi.number_entry != 0xffff)    /* 2^16 files overflow hack */
      if (s->num_file+1==s->gi.number_entry)
        return UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
            s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
    s->num_file++;
    err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               NULL,0,NULL,0,NULL,0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzipStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
int unzLocateFile (unzFile file,
				   const char *szFileName,
				   int iCaseSensitivity)
{
    unz_s* s;
    int err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info cur_file_infoSaved;
    unz_file_info_internal cur_file_info_internalSaved;
    uLong num_fileSaved;
    uLong pos_in_central_dirSaved;


    if (file==NULL)
        return UNZ_PARAMERROR;

    if (strlen(szFileName)>=UNZ_MAXFILENAMEINZIP)
        return UNZ_PARAMERROR;

    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    /* Save the current state */
    num_fileSaved = s->num_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_internalSaved = s->cur_file_info_internal;

    err = unzGoToFirstFile(file);

    while (err == UNZ_OK)
    {
        char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo(file,NULL,
                                    szCurrentFileName,sizeof(szCurrentFileName)-1,
                                    NULL,0,NULL,0);
        if (err == UNZ_OK)
        {
            if (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,iCaseSensitivity)==0)
                return UNZ_OK;
            err = unzGoToNextFile(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->num_file = num_fileSaved ;
    s->pos_in_central_dir = pos_in_central_dirSaved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_internal = cur_file_info_internalSaved;
    return err;
}



///////////////////////////////////////////
// Contributed by Ryan Haksi (mailto://cryogen@infoserve.net)
// I need random access
//
// Further optimization could be realized by adding an ability
// to cache the directory in memory. The goal being a single
// comprehensive file read to put the file I need in a memory.
//	typedef struct unz_file_pos_s
//	{
//		uLong pos_in_zip_directory;   // offset in file
//		uLong num_of_file;            // # of file
//	} unz_file_pos;

int unzGetFilePos(unzFile file, unz_file_pos* file_pos)
{
    unz_s* s;

    if (file==NULL || file_pos==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    file_pos->pos_in_zip_directory  = s->pos_in_central_dir;
    file_pos->num_of_file           = s->num_file;

    return UNZ_OK;
}

int unzGoToFilePos(unzFile file, unz_file_pos* file_pos)
{
    unz_s* s;
    int err;

    if (file==NULL || file_pos==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    /* jump to the right spot */
    s->pos_in_central_dir = file_pos->pos_in_zip_directory;
    s->num_file           = file_pos->num_of_file;

    /* set the current file */
    err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               NULL,0,NULL,0,NULL,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

/*
// Unzip Helper Functions - should be here?
///////////////////////////////////////////
*/

/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
        directory about this file
  store in *piSizeVar the size of extra info in local header
        (filename and size of extra field data)
*/
local int unzlocal_CheckCurrentFileCoherencyHeader (unz_s* s,
													uInt* piSizeVar,
                                                    uLong *poffset_local_extrafield,
                                                    uInt  *psize_local_extrafield)
{
    uLong uMagic,uData,uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int err=UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    if (ZSEEK(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET)!=0)
        return UNZ_ERRNO;


    if (err==UNZ_OK)
        if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        else if (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    if ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;


    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&size_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
        err=UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    if (unzlocal_getShort(&s->z_filefunc, s->filestream,&size_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
int unzOpenCurrentFile3 (unzFile file, 
										int* method, 
										int* level, 
										int raw, 
										const char* password)
{
    int err=UNZ_OK;
    uInt iSizeVar;
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    uLong offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    char source[12];
#    else
    if (password != NULL)
        return UNZ_PARAMERROR;
#    endif

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_PARAMERROR;

    if (s->pfile_in_zip_read != NULL)
        unzCloseCurrentFile(file);

    if (unzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar,
                &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
        return UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip_read_info_s*)
                                        ALLOC(sizeof(file_in_zip_read_info_s));
    if (pfile_in_zip_read_info==NULL)
        return UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_buffer=(char*)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;
    pfile_in_zip_read_info->raw=raw;

    if (pfile_in_zip_read_info->read_buffer==NULL)
    {
        TRYFREE(pfile_in_zip_read_info);
        return UNZ_INTERNALERROR;
    }

    pfile_in_zip_read_info->stream_initialised=0;

    if (method!=NULL)
        *method = (int)s->cur_file_info.compression_method;

    if (level!=NULL)
    {
        *level = 6;
        switch (s->cur_file_info.flag & 0x06)
        {
          case 6 : *level = 1; break;
          case 4 : *level = 2; break;
          case 2 : *level = 9; break;
        }
    }

    if ((s->cur_file_info.compression_method!=0) &&
        (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
    pfile_in_zip_read_info->crc32=0;
    pfile_in_zip_read_info->compression_method =
            s->cur_file_info.compression_method;
    pfile_in_zip_read_info->filestream=s->filestream;
    pfile_in_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

    if ((s->cur_file_info.compression_method==Z_DEFLATED) &&
        (!raw))
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)NULL;
      pfile_in_zip_read_info->stream.zfree = (free_func)NULL;
      pfile_in_zip_read_info->stream.opaque = NULL;
      pfile_in_zip_read_info->stream.next_in = NULL;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
      if (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=1;
      else
      {
        TRYFREE(pfile_in_zip_read_info);
        return err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_in_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_size ;
    pfile_in_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_size ;


    pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
              iSizeVar;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;

    s->pfile_in_zip_read = pfile_in_zip_read_info;

#    ifndef NOUNCRYPT
    if (password != NULL)
    {
        int i;
        s->pcrc_32_tab = get_crc_table();
        init_keys(password,s->keys,s->pcrc_32_tab);
        if (ZSEEK(s->z_filefunc, s->filestream,
                  s->pfile_in_zip_read->pos_in_zipfile +
                     s->pfile_in_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            return UNZ_INTERNALERROR;
        if(ZREAD(s->z_filefunc, s->filestream,source, 12)<12)
            return UNZ_INTERNALERROR;

        for (i = 0; i<12; i++)
            zdecode(s->keys,s->pcrc_32_tab,source[i]);

        s->pfile_in_zip_read->pos_in_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    return UNZ_OK;
}

int unzOpenCurrentFile (unzFile file)
{
    return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

int unzOpenCurrentFilePassword(unzFile file, const char* password)
{
    return unzOpenCurrentFile3(file, NULL, NULL, 0, password);
}

int unzOpenCurrentFile2 (unzFile file,
						 int* method,
						 int* level,
						 int raw)
{
    return unzOpenCurrentFile3(file, method, level, raw, NULL);
}

/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
int unzReadCurrentFile(unzFile file, voidp buf, unsigned len)
{
    int err=UNZ_OK;
    uInt iRead = 0;
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;


    if ((pfile_in_zip_read_info->read_buffer == NULL))
        return UNZ_END_OF_LIST_OF_FILE;
    if (len==0)
        return 0;

    pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    if ((len>pfile_in_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    if ((len>pfile_in_zip_read_info->rest_read_compressed+
           pfile_in_zip_read_info->stream.avail_in) &&
         (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_compressed+
            pfile_in_zip_read_info->stream.avail_in;

    while (pfile_in_zip_read_info->stream.avail_out>0)
    {
        if ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            if (uReadThis == 0)
                return UNZ_EOF;
            if (ZSEEK(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_SET)!=0)
                return UNZ_ERRNO;
            if (ZREAD(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->read_buffer,
                      uReadThis)!=uReadThis)
                return UNZ_ERRNO;


#            ifndef NOUNCRYPT
            if(s->encrypted)
            {
                uInt i;
                for(i=0;i<uReadThis;i++)
                  pfile_in_zip_read_info->read_buffer[i] =
                      zdecode(s->keys,s->pcrc_32_tab,
                              pfile_in_zip_read_info->read_buffer[i]);
            }
#            endif


            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_in_zip_read_info->stream.next_in =
                (Bytef*)pfile_in_zip_read_info->read_buffer;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        if ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            if ((pfile_in_zip_read_info->stream.avail_in == 0) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                return (iRead==0) ? UNZ_EOF : iRead;

            if (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            else
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            for (i=0;i<uDoCopy;i++)
                *(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                pfile_in_zip_read_info->stream.next_out,
                                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.next_out += uDoCopy;
            pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        }
        else
        {
            uLong uTotalOutBefore,uTotalOutAfter;
            const Bytef *bufBefore;
            uLong uOutThis;
            int flush=Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_in_zip_read_info->stream,flush);

            if ((err>=0) && (pfile_in_zip_read_info->stream.msg!=NULL))
              err = Z_DATA_ERROR;

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            if (err==Z_STREAM_END)
                return (iRead==0) ? UNZ_EOF : iRead;
            if (err!=Z_OK)
                break;
        }
    }

    if (err==Z_OK)
        return iRead;
    return err;
}


/*
  Give the current position in uncompressed data
*/
z_off_t unztell (unzFile file)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
int unzeof (unzFile file)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        return 1;
    else
        return 0;
}



/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
    more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/
int unzGetLocalExtrafield (unzFile file, voidp buf, unsigned len)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    uInt read_now;
    uLong size_to_read;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
                pfile_in_zip_read_info->pos_local_extrafield);

    if (buf==NULL)
        return (int)size_to_read;

    if (len>size_to_read)
        read_now = (uInt)size_to_read;
    else
        read_now = (uInt)len ;

    if (read_now==0)
        return 0;

    if (ZSEEK(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              pfile_in_zip_read_info->offset_local_extrafield +
              pfile_in_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        return UNZ_ERRNO;

    if (ZREAD(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              buf,read_now)!=read_now)
        return UNZ_ERRNO;

    return (int)read_now;
}

/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
int unzCloseCurrentFile (unzFile file)
{
    int err=UNZ_OK;

    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;


    if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_in_zip_read_info->raw))
    {
        if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err=UNZ_CRCERROR;
    }


    TRYFREE(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = NULL;
    if (pfile_in_zip_read_info->stream_initialised)
        inflateEnd(&pfile_in_zip_read_info->stream);

    pfile_in_zip_read_info->stream_initialised = 0;
    TRYFREE(pfile_in_zip_read_info);

    s->pfile_in_zip_read=NULL;

    return err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
int unzGetGlobalComment (unzFile file, char *szComment, uLong uSizeBuf)
{
    unz_s* s;
    uLong uReadThis ;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    uReadThis = uSizeBuf;
    if (uReadThis>s->gi.size_comment)
        uReadThis = s->gi.size_comment;

    if (ZSEEK(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_SET)!=0)
        return UNZ_ERRNO;

    if (uReadThis>0)
    {
      *szComment='\0';
      if (ZREAD(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        return UNZ_ERRNO;
    }

    if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment))
        *(szComment+s->gi.size_comment)='\0';
    return (int)uReadThis;
}

/* Additions by RX '2004 */
uLong unzGetOffset (unzFile file)
{
    unz_s* s;

    if (file==NULL)
          return 0;
    s=(unz_s*)file;
    if (!s->current_file_ok)
      return 0;
    if (s->gi.number_entry != 0 && s->gi.number_entry != 0xffff)
      if (s->num_file==s->gi.number_entry)
         return 0;
    return s->pos_in_central_dir;
}

int unzSetOffset (unzFile file, uLong pos)
{
    unz_s* s;
    int err;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    s->pos_in_central_dir = pos;
    s->num_file = s->gi.number_entry;      /* hack */
    err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                              &s->cur_file_info_internal,
                                              NULL,0,NULL,0,NULL,0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}




/*
   miniunz.c
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
# include <direct.h>
# include <io.h>
#endif


#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
#define USEWIN32IOAPI


#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

voidpf win32_open_file_func (
   voidpf opaque,
   const char* filename,
   int mode);

uLong win32_read_file_func (
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size);

uLong win32_write_file_func (
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size);

long win32_tell_file_func (
   voidpf opaque,
   voidpf stream);

long win32_seek_file_func (
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin);

int win32_close_file_func(
   voidpf opaque,
   voidpf stream);

int win32_error_file_func (
   voidpf opaque,
   voidpf stream);

typedef struct
{
    HANDLE hf;
    int error;
} WIN32FILE_IOWIN;

voidpf win32_open_file_func (voidpf opaque, const char* filename, int mode)
{
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = 0;
    voidpf ret=NULL;

    dwCreationDisposition=dwDesiredAccess = dwShareMode = dwFlagsAndAttributes = 0;

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        dwDesiredAccess = GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
        dwShareMode = FILE_SHARE_READ;
    }
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
    }
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        dwCreationDisposition = CREATE_ALWAYS;
    }

    if ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL,
                      dwCreationDisposition, dwFlagsAndAttributes, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        hFile = NULL;

    if (hFile != NULL)
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(sizeof(WIN32FILE_IOWIN));
        if (ret==NULL)
            CloseHandle(hFile);
        else *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    return ret;
}


uLong win32_read_file_func (voidpf opaque, voidpf stream, void* buf, uLong size)
{
    uLong ret=0;
    HANDLE hFile = NULL;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    if (hFile != NULL)
        if (!ReadFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
        }

    return ret;
}


uLong win32_write_file_func (voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    uLong ret=0;
    HANDLE hFile = NULL;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    if (hFile !=NULL)
        if (!WriteFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
        }

    return ret;
}

long win32_tell_file_func (voidpf opaque, voidpf stream)
{
    long ret=-1;
    HANDLE hFile = NULL;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    if (hFile != NULL)
    {
        DWORD dwSet = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
        if (dwSet == INVALID_SET_FILE_POINTER)
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
            ret = -1;
        }
        else
            ret=(long)dwSet;
    }
    return ret;
}

long win32_seek_file_func (voidpf opaque, voidpf stream, uLong offset, int origin)
{
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;

    long ret=-1;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        break;
    default: return -1;
    }

    if (hFile != NULL)
    {
        DWORD dwSet = SetFilePointer(hFile, offset, NULL, dwMoveMethod);
        if (dwSet == INVALID_SET_FILE_POINTER)
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
            ret = -1;
        }
        else
            ret=0;
    }
    return ret;
}

int win32_close_file_func (voidpf opaque, voidpf stream)
{
    int ret=-1;

    if (stream!=NULL)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        if (hFile != NULL)
        {
            CloseHandle(hFile);
            ret=0;
        }
        free(stream);
    }
    return ret;
}

int win32_error_file_func (voidpf opaque, voidpf stream)
{
    int ret=-1;
    if (stream!=NULL)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    return ret;
}

void fill_win32_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque=NULL;
}

#endif

/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char *filename,
					  uLong dosdate,
					  tm_unz tmu_date)
{
#ifdef WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
#ifdef unix
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

int mymkdir(const char* dirname)
{
    int ret=0;
#ifdef WIN32
    ret = mkdir(dirname);
#else
#ifdef unix
    ret = mkdir (dirname,0775);
#endif
#endif
    return ret;
}

int makedir (const char *newdir)
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}

void do_banner()
{
    printf("MiniUnz 1.01b, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at http://www.winimage.com/zLibDll/unzip.html\n\n");
}

void do_help()
{
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract crypted file using password\n\n");
}


int do_list(unzFile uf)
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    printf(" Length  Method   Size  Ratio   Date    Time   CRC-32     Name\n");
    printf(" ------  ------   ----  -----   ----    ----   ------     ----\n");
    for (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[256];
        unz_file_info file_info;
        uLong ratio=0;
        const char *string_method="";
        char charCrypt=' ';
        err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        if (file_info.uncompressed_size>0)
            ratio = (file_info.compressed_size*100)/file_info.uncompressed_size;

        /* display a '*' if the file is crypted */
        if ((file_info.flag & 1) != 0)
            charCrypt='*';

        if (file_info.compression_method==0)
            string_method="Stored";
        else
        if (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            if (iLevel==0)
              string_method="Defl:N";
            else if (iLevel==1)
              string_method="Defl:X";
            else if ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        else
            string_method="Unkn. ";

        printf("%7lu  %6s%c%7lu %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                file_info.uncompressed_size,string_method,
                charCrypt,
                file_info.compressed_size,
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}


int do_extract_currentfile(unzFile uf,
						   const int* popt_extract_without_path,
						   int* popt_overwrite,
						   const char* password)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    if (ret != 1) 
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fout=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }

            if (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        if (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}


int do_extract(unzFile uf,
			   int opt_extract_without_path,
			   int opt_overwrite,
			   const char* password)
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int do_extract_onefile(unzFile uf,
					   const char* filename,
					   int opt_extract_without_path,
					   int opt_overwrite,
					   const char* password)
{
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) == UNZ_OK)
        return 0;
    else
        return 1;
}



