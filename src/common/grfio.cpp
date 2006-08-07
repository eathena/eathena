/*********************************************************************
 *
 *		Ragnarok Online Emulator : grfio.c -- grf file I/O Module
 *--------------------------------------------------------------------
 *		special need library : zlib
 *********************************************************************
 *	$Id: grfio.c,v 1.2 2004/09/29 17:31:49 kalaspuff Exp $
 *
 *	2002/12/18... the original edition
 *	2003/01/23 ... Code correction
 *	2003/02/01 ... An addition and decryption processing are improved for LocalFile and two or more GRF(s) check processing.
 *	2003/02/02 ... Even if there is no grf it does not stop -- as -- correction
 *	2003/02/02... grf reading specification can be added later -- as -- correction (grfio_add function addition)
 *	2003/02 / 03... at the time of grfio_resourcecheck processing the entry addition processing method -- correction
 *	2003/02/05... change of the processing in grfio_init
 *	2003/02/23... a local file check -- GRFIO_LOCAL -- switch (Defoe -- Function Off)
 *      2003/10/21 ... The data of alpha client was read.
 *	2003/11/10 ... Ready new grf format.
 *	2003/11/11 ... version check fix & bug fix
 */

#include "grfio.h"
#include "utils.h"
#include "malloc.h"
#include "showmsg.h"

#include <zlib.h>



static char data_dir[1024] = "";	// "../";

//static char data_file[1024] = "";	// "data.grf";
//static char sdata_file[1024] = "";	// "sdata.grf";
//static char adata_file[1024] = "";	// "adata.grf";

//----------------------------
//	file entry table struct
//----------------------------
typedef struct {
	long 	srclen;				// compressed size
	long	srclen_aligned;		//
	long	declen;				// original size
	long	srcpos;				// position in file
	long	nexthash;			// next hash in hashrow
	char	cycle;				// des encryption cycle
	char	type;				// encoding type
	char	gentry;				// read grf file select
	char	fn[128-4*5-3];		// file name
} FILELIST;
//gentry ... 0    : It acquires from a local file.
//             It acquires from the resource file of 1>=:gentry_table[gentry-1].
//             1<=: Check a local file.
//                    If it is, after re-setting to 0, it acquires from a local file.
//                    If there is nothing, mark reversal will be carried out, and it will re-set, and will acquire from a resource file as well as 1>=.

//Since char defines *FILELIST.gentry, the maximum which can be added by grfio_add becomes by 127 pieces.

#define	GENTRY_LIMIT	127
#define	FILELIST_LIMIT	65536	// temporary maximum, and a theory top maximum are 2G.

FILELIST *filelist=NULL;
size_t	filelist_entrys=0;
size_t	filelist_maxentry=0;

char **gentry_table=NULL;
size_t gentry_entrys=0;
size_t gentry_maxentry=0;

//----------------------------
//	file list hash table
//----------------------------
long filelist_hash[256];

//----------------------------
//	grf decode data table
//----------------------------
static unsigned char BitMaskTable[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static unsigned char	BitSwapTable1[64] = {
	58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15,  7
};
static unsigned char	BitSwapTable2[64] = {
	40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};
static unsigned char	BitSwapTable3[32] = {
	16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

static unsigned char NibbleData[4][64]={
	{
		0xef, 0x03, 0x41, 0xfd, 0xd8, 0x74, 0x1e, 0x47,  0x26, 0xef, 0xfb, 0x22, 0xb3, 0xd8, 0x84, 0x1e,
		0x39, 0xac, 0xa7, 0x60, 0x62, 0xc1, 0xcd, 0xba,  0x5c, 0x96, 0x90, 0x59, 0x05, 0x3b, 0x7a, 0x85,
		0x40, 0xfd, 0x1e, 0xc8, 0xe7, 0x8a, 0x8b, 0x21,  0xda, 0x43, 0x64, 0x9f, 0x2d, 0x14, 0xb1, 0x72,
		0xf5, 0x5b, 0xc8, 0xb6, 0x9c, 0x37, 0x76, 0xec,  0x39, 0xa0, 0xa3, 0x05, 0x52, 0x6e, 0x0f, 0xd9,
	}, {
		0xa7, 0xdd, 0x0d, 0x78, 0x9e, 0x0b, 0xe3, 0x95,  0x60, 0x36, 0x36, 0x4f, 0xf9, 0x60, 0x5a, 0xa3,
		0x11, 0x24, 0xd2, 0x87, 0xc8, 0x52, 0x75, 0xec,  0xbb, 0xc1, 0x4c, 0xba, 0x24, 0xfe, 0x8f, 0x19,
		0xda, 0x13, 0x66, 0xaf, 0x49, 0xd0, 0x90, 0x06,  0x8c, 0x6a, 0xfb, 0x91, 0x37, 0x8d, 0x0d, 0x78,
		0xbf, 0x49, 0x11, 0xf4, 0x23, 0xe5, 0xce, 0x3b,  0x55, 0xbc, 0xa2, 0x57, 0xe8, 0x22, 0x74, 0xce,
	}, {
		0x2c, 0xea, 0xc1, 0xbf, 0x4a, 0x24, 0x1f, 0xc2,  0x79, 0x47, 0xa2, 0x7c, 0xb6, 0xd9, 0x68, 0x15,
		0x80, 0x56, 0x5d, 0x01, 0x33, 0xfd, 0xf4, 0xae,  0xde, 0x30, 0x07, 0x9b, 0xe5, 0x83, 0x9b, 0x68,
		0x49, 0xb4, 0x2e, 0x83, 0x1f, 0xc2, 0xb5, 0x7c,  0xa2, 0x19, 0xd8, 0xe5, 0x7c, 0x2f, 0x83, 0xda,
		0xf7, 0x6b, 0x90, 0xfe, 0xc4, 0x01, 0x5a, 0x97,  0x61, 0xa6, 0x3d, 0x40, 0x0b, 0x58, 0xe6, 0x3d,
	}, {
		0x4d, 0xd1, 0xb2, 0x0f, 0x28, 0xbd, 0xe4, 0x78,  0xf6, 0x4a, 0x0f, 0x93, 0x8b, 0x17, 0xd1, 0xa4,
		0x3a, 0xec, 0xc9, 0x35, 0x93, 0x56, 0x7e, 0xcb,  0x55, 0x20, 0xa0, 0xfe, 0x6c, 0x89, 0x17, 0x62,
		0x17, 0x62, 0x4b, 0xb1, 0xb4, 0xde, 0xd1, 0x87,  0xc9, 0x14, 0x3c, 0x4a, 0x7e, 0xa8, 0xe2, 0x7d,
		0xa0, 0x9f, 0xf6, 0x5c, 0x6a, 0x09, 0x8d, 0xf0,  0x0f, 0xe3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xcb,
	}
};
/*-----------------
 *	long data get
 */
uint32 getlong(unsigned char *p)
{
//	return *p+p[1]*256+(p[2]+p[3]*256)*65536;
	return    p[0]
			| p[1] << 0x08
			| p[2] << 0x10
			| p[3] << 0x18;
}

/*==========================================
 *	Grf data decode : Subs
 *------------------------------------------
 */
void NibbleSwap(unsigned char *Src, size_t len)
{
	for(;0<len;len--,Src++) {
		*Src = (*Src>>4) | (*Src<<4);
	}
}

void BitConvert(unsigned char *Src,unsigned char *BitSwapTable)
{
	size_t lop,prm;
	unsigned char tmp[8];
	memset(tmp,0,8);
	for(lop=0;lop!=64;++lop) {
		prm = BitSwapTable[lop]-1;
		if (Src[(prm >> 3) & 7] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) & 7] |= BitMaskTable[lop & 7];
		}
	}
	memcpy(Src,tmp,8);
}

void BitConvert4(unsigned char *Src)
{
	size_t lop,prm;
	unsigned char tmp[8];
	tmp[0] = ((Src[7]<<5) | (Src[4]>>3)) & 0x3f;	// ..0 vutsr
	tmp[1] = ((Src[4]<<1) | (Src[5]>>7)) & 0x3f;	// ..srqpo n
	tmp[2] = ((Src[4]<<5) | (Src[5]>>3)) & 0x3f;	// ..o nmlkj
	tmp[3] = ((Src[5]<<1) | (Src[6]>>7)) & 0x3f;	// ..kjihg f
	tmp[4] = ((Src[5]<<5) | (Src[6]>>3)) & 0x3f;	// ..g fedcb
	tmp[5] = ((Src[6]<<1) | (Src[7]>>7)) & 0x3f;	// ..cba98 7
	tmp[6] = ((Src[6]<<5) | (Src[7]>>3)) & 0x3f;	// ..8 76543
	tmp[7] = ((Src[7]<<1) | (Src[4]>>7)) & 0x3f;	// ..43210 v

	for(lop=0;lop!=4;++lop) {
		tmp[lop] = (NibbleData[lop][tmp[lop*2]] & 0xf0)
		         | (NibbleData[lop][tmp[lop*2+1]] & 0x0f);
	}

	memset(tmp+4,0,4);
	for(lop=0;lop!=32;++lop) {
		prm = BitSwapTable3[lop]-1;
		if (tmp[prm >> 3] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) + 4] |= BitMaskTable[lop & 7];
		}
	}
	Src[0] ^= tmp[4];
	Src[1] ^= tmp[5];
	Src[2] ^= tmp[6];
	Src[3] ^= tmp[7];
}

void decode_des_etc(unsigned char *buf, size_t len, int type, size_t cycle)
{
	size_t lop,cnt=0;
	if(cycle<3) cycle=3;
	else if(cycle<5) cycle++;
	else if(cycle<7) cycle+=9;
	else cycle+=15;

	for(lop=0;lop*8<len;lop++,buf+=8) {
		if(lop<20 || (type==0 && lop%cycle==0)){ // des
			BitConvert(buf,BitSwapTable1);
			BitConvert4(buf);
			BitConvert(buf,BitSwapTable2);
		} else {
			if(cnt==7 && type==0){
				size_t a;
				unsigned char tmp[8];
				memcpy(tmp,buf,8);
				cnt=0;
				buf[0]=tmp[3];
				buf[1]=tmp[4];
				buf[2]=tmp[6];
				buf[3]=tmp[0];
				buf[4]=tmp[1];
				buf[5]=tmp[2];
				buf[6]=tmp[5];
				a=tmp[7];
				if(a==0x00) a=0x2b;
				else if(a==0x2b) a=0x00;
				else if(a==0x01) a=0x68;
				else if(a==0x68) a=0x01;
				else if(a==0x48) a=0x77;
				else if(a==0x77) a=0x48;
				else if(a==0x60) a=0xff;
				else if(a==0xff) a=0x60;
				else if(a==0x6c) a=0x80;
				else if(a==0x80) a=0x6c;
				else if(a==0xb9) a=0xc0;
				else if(a==0xc0) a=0xb9;
				else if(a==0xeb) a=0xfe;
				else if(a==0xfe) a=0xeb;
				buf[7]=a;
			}
			cnt++;
		}
	}
}
/*==========================================
 *	Grf data decode sub : zip
 *------------------------------------------
 */
int decode_zip(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((unsigned long)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}

int encode_zip(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((uLong)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = deflateInit(&stream,Z_DEFAULT_COMPRESSION);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}
/*==========================================
*  Decompress from file source to file dest until stream ends or EOF.
*  inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
*  allocated for processing, Z_DATA_ERROR if the deflate data is
*  invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
*  the version of the library linked do not match, or Z_ERRNO if there
*  is an error reading or writing the files.
*
*  Version 1.2  9 November 2004  Mark Adler
*------------------------------------------
*/

#define CHUNK 16384

int decode_file (FILE *source, FILE *dest)
{
	int err;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	err = inflateInit(&strm);
	if (err != Z_OK) return 0;	//return err;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			inflateEnd(&strm);
			return 0;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			err = inflate(&strm, Z_NO_FLUSH);
			assert(err != Z_STREAM_ERROR);  /* state not clobbered */
			switch (err) {
			case Z_NEED_DICT:
				err = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&strm);
				//return err;
				return 0;
			}
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				inflateEnd(&strm);
				//return Z_ERRNO;
				return 0;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		/* done when inflate() says it's done */
	} while (err != Z_STREAM_END);

	/* clean up and return */
	inflateEnd(&strm);
	return err == Z_STREAM_END ? 1 : 0;
}

/***********************************************************
 ***                File List Sobroutines                ***
 ***********************************************************/

/*==========================================
 *	File List : Hash make
 *------------------------------------------
 */
unsigned char filehash(unsigned char *fname)
{
	size_t hash=0;
	while(*fname) {
		hash = ((hash<<1)+(hash>>7)*9+tolower(*fname));
		fname++;
	}
	return hash & 0xFF;
}

/*==========================================
 *	File List : Hash initalize
 *------------------------------------------
 */
void hashinit(void)
{
	size_t lop;
	for(lop=0;lop<256;++lop)
		filelist_hash[lop]=-1;
}

/*==========================================
 *	File List : File find
 *------------------------------------------
 */

bool debug = false;
FILELIST *filelist_find(const char *fname)
{
	int hash;
	char *p;
	char namebuffer[2048];

	if(!fname || !filelist)
		return NULL;

	// grf files uses backslash seperator
	// so we better check if there are backslashes used
	// combine it with the copy, we cannot write to a const char 
	// without crashing on "real" machines
	// fixed buffer length should be ok here
	p = namebuffer;
	while(*fname) {
		if (*fname=='/') {
			*p++ = '\\';
			fname++;
		}
		else
			*p++ = *fname++;
	}
	*p = 0; //EOS

	for(hash=filelist_hash[filehash((unsigned char *)namebuffer)];hash>=0;hash=filelist[hash].nexthash) {
		if(strcasecmp(filelist[hash].fn,namebuffer)==0)
			break;
	}
	return (hash>=0)? &filelist[hash] : NULL;
}

/*==========================================
 *	File List : Filelist add
 *------------------------------------------
 */
#define	FILELIST_ADDS	1024	// number increment of file lists `

FILELIST* filelist_add(FILELIST *entry)
{
	int hash;

	if (filelist_entrys>=FILELIST_LIMIT) {
		ShowFatalError("filelist limit : filelist_add\n");
		exit(1);
	}

	if (filelist_entrys>=filelist_maxentry)
	{
		filelist_maxentry = new_realloc(filelist, filelist_maxentry, FILELIST_ADDS);
	}

	memcpy( &filelist[filelist_entrys], entry, sizeof(FILELIST) );

	hash = filehash((unsigned char*)(entry->fn));
	filelist[filelist_entrys].nexthash = filelist_hash[hash];
	filelist_hash[hash] = filelist_entrys;

	filelist_entrys++;

	return &filelist[filelist_entrys-1];
}

FILELIST* filelist_modify(FILELIST *entry)
{
	FILELIST *fentry;
	if ((fentry=filelist_find(entry->fn))!=NULL)
	{
		int tmp = fentry->nexthash;
		memcpy( fentry, entry, sizeof(FILELIST) );
		fentry->nexthash = tmp;
	}
	else
	{
		fentry = filelist_add(entry);
	}
	return fentry;
}

/*==========================================
 *	File List : filelist size adjust
 *------------------------------------------
 */
void filelist_adjust(void)
{
	if (filelist!=NULL && filelist_maxentry>filelist_entrys)
	{
		FILELIST* old = filelist;
		if(filelist_entrys)
		{
			filelist = new FILELIST[filelist_entrys];
			memcpy(filelist, old, filelist_entrys*sizeof(FILELIST));
		}
		else
			filelist = NULL;

		filelist_maxentry = filelist_entrys;
		delete[] old;
	}
}

/***********************************************************
 ***                  Grfio Sobroutines                  ***
 ***********************************************************/
/*==========================================
 * Grfio : Resnametable replace
 *------------------------------------------
 */
const char* grfio_resnametable(const char* fname, char *lfname, size_t sz)
{
   	FILE *fp;
	char w1[256],w2[256],restable[256],line[512];

	snprintf(restable,sizeof(restable), "%sdata\\resnametable.txt", data_dir);
	fp = basics::safefopen(restable,"rb");
	if(fp==NULL)
	{
		ShowError("%s not found (grfio_resnametable)\n", restable);
		*lfname=0;
		return lfname;
		// returning NULL will crash the following sprintf
		//return NULL;	// 1:not found error, 
	}

	while(fgets(line,sizeof(line),fp))
	{
		if( (sscanf(line,"%[^#]#%[^#]#",w1,w2)==2) && 
			(sscanf(fname,"%*5s%s",lfname)==1) &&  
			(!strcasecmp(w1,lfname)))
		{
			snprintf(lfname,sz,"%sdata\\%s",data_dir,w2);
			fclose(fp);
			return lfname;
		}
    }
    fclose(fp);
	snprintf(lfname, sz, "%s%s", data_dir, fname);
    return fname;
}

/*==========================================
 *	Grfio : Resource file size get
 *------------------------------------------
 */
int grfio_size(const char *fname)
{
	FILELIST *entry;

	entry = filelist_find(fname);

	if (entry==NULL || entry->gentry<0) {	// LocalFileCheck
		char lfname[256]="";
		FILELIST lentry;
		struct stat st;

		if(strcmp(data_dir, "") != 0)
		{
			grfio_resnametable(fname,lfname, sizeof(lfname));
        }

		if(stat(lfname,&st)==0) {
			safestrcpy(lentry.fn, fname, sizeof(lentry.fn)-1 );
			lentry.declen = st.st_size;
			lentry.gentry = 0;	// 0:LocalFile
			entry = filelist_modify(&lentry);
		} else if (entry==NULL) {
			ShowError("%s not found (grfio_size)\n", fname);
         	return -1;
		}
	}
	return entry->declen;
}

/*==========================================
 *	Grfio : Resource file read & size get
 *------------------------------------------
 */
unsigned char* grfio_reads(const char *fname, int &size)
{
	FILE *in = NULL;
	unsigned char *buf=NULL,*buf2=NULL;
	char *gfname;
	FILELIST *entry;

	entry = filelist_find(fname);

	if (entry==NULL || entry->gentry<=0)
	{	// LocalFileCheck
		char lfname[256];
		FILELIST lentry;
		safestrcpy(lfname,fname,sizeof(lfname));
		grfio_resnametable(fname,lfname, sizeof(lfname));
		
		if(*lfname)
			in = basics::safefopen(lfname,"rb");
		if(in!=NULL)
		{
			if (entry!=NULL && entry->gentry==0)
			{
				lentry.declen=entry->declen;
			}
			else
			{
				fseek(in,0,2);	// SEEK_END
				lentry.declen = ftell(in);
			}
			fseek(in,0,0);	// SEEK_SET
			
			buf2 = new unsigned char[lentry.declen+1024];

			if (buf2==NULL)
			{
				ShowError("file read memory allocate error : declen\n");
				goto errret;
			}
			fread(buf2,1,lentry.declen,in);
			fclose(in); in = NULL;
			safestrcpy( lentry.fn, fname, sizeof(lentry.fn)-1 );
			lentry.gentry = 0;	// 0:LocalFile
			entry = filelist_modify(&lentry);
		}
		else
		{
			if (entry!=NULL && entry->gentry<0)
			{
				entry->gentry = -entry->gentry;	// local file checked
			}
			else
			{
				ShowError("%s not found (grfio_reads)\n", fname);
				//goto errret;
				return NULL;
			}
		}
	}
	else //if (entry!=NULL && entry->gentry>0)
	{	// Archive[GRF] File Read
		buf = new unsigned char[entry->srclen_aligned+1024];
		if (buf==NULL)
		{
			ShowError("file read memory allocate error : srclen_aligned\n");
			goto errret;
		}
		gfname = gentry_table[entry->gentry-1];
		in = basics::safefopen(gfname,"rb");
		if(in==NULL)
		{
			ShowError("%s not found (grfio_reads)\n",gfname);
			//goto errret;
			delete[] buf;
			return NULL;
		}
		fseek(in,entry->srcpos,0);
		fread(buf,1,entry->srclen_aligned,in);
		fclose(in);

		buf2 = new unsigned char[entry->declen+1024];
		if(buf2==NULL)
		{
			ShowError("file decode memory allocate error\n");
			goto errret;
		}
		if(entry->type==1 || entry->type==3 || entry->type==5) {
			unsigned long len;
			int err;
			if (entry->cycle>=0) {
				decode_des_etc(buf,entry->srclen_aligned,entry->cycle==0,entry->cycle);
			}
			len=entry->declen;
			err=decode_zip(buf2,len,buf,entry->srclen);

			if((long)len!=entry->declen) {
				ShowError("decode_zip size miss match err: %d != %d\n", len,entry->declen);
				goto errret;
			}
			if(err<0) {
				ShowMessage("decode_zip error %i!\n",err);
				goto errret;
			}
		}
		else
		{
			memcpy(buf2,buf,entry->declen);
		}
		if(buf) delete[] buf;
	}
	if(entry!=NULL)
		size = entry->declen;
	return buf2;
errret:
	if(buf!=NULL) delete[] buf;
	if(buf2!=NULL) delete[] buf2;
	if(in!=NULL) fclose(in);
	return NULL;
}

/*==========================================
 *	Grfio : Resource file read
 *------------------------------------------
 */
unsigned char* grfio_read(const char *fname)
{
	int sz;
	return grfio_reads(fname, sz);
}

/*==========================================
 *	Resource filename decode
 *------------------------------------------
 */
char * decode_filename(unsigned char *buf,int len)
{
	int lop;
	for(lop=0;lop<len;lop+=8) {
		NibbleSwap(&buf[lop],8);
		BitConvert(&buf[lop],BitSwapTable1);
		BitConvert4(&buf[lop]);
		BitConvert(&buf[lop],BitSwapTable2);
	}
	return (char*)buf;
}

/*==========================================
 * Grfio : Entry table read
 *------------------------------------------
 */
int grfio_entryread(const char *gfname, int gentry)
{
	FILE *fp;
	long grf_size,list_size;
	unsigned char grf_header[0x2e];
	ssize_t lop,entry,entrys,ofs,grf_version;
	char *fname;
	unsigned char *grf_filelist;

	fp = basics::safefopen(gfname,"rb");
	if(fp==NULL) {
		ShowWarning("GRF Data File not found: '"CL_WHITE"%s"CL_RESET"'.\n",gfname);
		return 1;	// 1:not found error
	}

	fseek(fp,0,SEEK_END);
	grf_size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(grf_header,1,0x2e,fp);
	if(strcmp((char*)grf_header,"Master of Magic") || fseek(fp,getlong(grf_header+0x1e),1))
	{	// SEEK_CUR
		fclose(fp);
		ShowError("%s read error\n",gfname);
		return 2;	// 2:file format error
	}

	grf_version = getlong(grf_header+0x2a) >> 8;

	if (grf_version==0x01)
	{	//****** Grf version 01xx ******
		list_size = grf_size-ftell(fp);
		grf_filelist = new unsigned char[list_size];
		if(grf_filelist==NULL)
		{
			fclose(fp);
			ShowError("out of memory : grf_filelist\n");
			return 3;	// 3:memory alloc error
		}
		fread(grf_filelist,1,list_size,fp);
		fclose(fp);

		entrys = getlong(grf_header+0x26) - getlong(grf_header+0x22) - 7;

		// Get an entry
		for(entry=0,ofs=0;entry<entrys;++entry)
		{
			int ofs2,srclen,srccount,type;
			char *period_ptr;
			FILELIST aentry;
			ofs2 = ofs+getlong(grf_filelist+ofs)+4;
			type = grf_filelist[ofs2+12];
			if( type!=0 )
			{	// Directory Index ... skip
				fname = decode_filename(grf_filelist+ofs+6, grf_filelist[ofs]-6);
				if(strlen(fname)>sizeof(aentry.fn)-1)
				{
					ShowFatalError("file name too long : %s\n",fname);
					delete[] grf_filelist;
					exit(1);
				}
				srclen=0;
				if((period_ptr=strrchr(fname,'.'))!=NULL)
				{
					for(lop=0;lop<4;++lop)
					{
						if(strcasecmp(period_ptr,".gnd\0.gat\0.act\0.str"+lop*5)==0)
							break;
					}
					srclen=getlong(grf_filelist+ofs2)-getlong(grf_filelist+ofs2+8)-715;
					if(lop==4)
					{
						for(lop=10,srccount=1;srclen>=lop;lop=lop*10,srccount++);
					}
					else
					{
						srccount=0;
					}
				}
				else
				{
					srccount=0;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4)-37579;
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				safestrcpy(aentry.fn,fname,sizeof(aentry.fn)-1);
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		delete[] grf_filelist;
	}
	else if (grf_version==0x02)
	{	//****** Grf version 02xx ******
		unsigned char eheader[8];
		unsigned char *rBuf;
		unsigned long rSize,eSize;
		fread(eheader,1,8,fp);
		rSize = getlong(eheader);	// Read Size
		eSize = getlong(eheader+4);	// Extend Size

		if((long)rSize > grf_size-ftell(fp))
		{
			fclose(fp);
			ShowError("Illegal data format : grf compress entry size\n");
			return 4;
		}

		rBuf = new unsigned char[rSize];
		if (rBuf==NULL) {
			fclose(fp);
			ShowError("out of memory : grf compress entry table buffer\n");
			return 3;
		}
		grf_filelist = new unsigned char[eSize];
		if (grf_filelist==NULL)
		{
			delete[] rBuf;
			fclose(fp);
			ShowError("out of memory : grf extract entry table buffer\n");
			return 3;
		}
		fread(rBuf,1,rSize,fp);
		fclose(fp);

		decode_zip(grf_filelist,eSize,rBuf,rSize);	// Decode function
		list_size = eSize;
		delete[] rBuf;

		entrys = getlong(grf_header+0x26) - 7;

		// Get an entry
		ssize_t ofs2,srclen,srccount,type;
		FILELIST aentry;


		for(entry=0,ofs=0;entry<entrys;++entry)
		{
			fname = (char*)(grf_filelist+ofs);
			if (strlen(fname)>sizeof(aentry.fn)-1) {
				ShowFatalError("grf : file name too long : %s\n",fname);
				delete[] grf_filelist;
				exit(1);
			}
			//ofs2 = ofs+strlen((char*)(grf_filelist+ofs))+1;
			ofs2 = ofs+strlen(fname)+1;
			type = grf_filelist[ofs2+12];
			if(type==1 || type==3 || type==5)
			{
				srclen=getlong(grf_filelist+ofs2);
				if (grf_filelist[ofs2+12]==3)
				{
					for(lop=10,srccount=1;srclen>=lop;lop=lop*10,srccount++);
				}
				else if (grf_filelist[ofs2+12]==5)
				{
					srccount = 0;
				}
				else// if (grf_filelist[ofs2+12]==1)
				{	
					srccount = -1;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4);
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				safestrcpy(aentry.fn, fname,sizeof(aentry.fn));
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		delete[] grf_filelist;

	}
	else
	{	//****** Grf Other version ******
		fclose(fp);
		ShowError("not support grf versions : %04x\n",getlong(grf_header+0x2a));
		return 4;
	}

	filelist_adjust();	// Unnecessary area release of filelist

	return 0;	// 0:no error
}

/*==========================================
 * Grfio : Resource file check
 *------------------------------------------
 */
void grfio_resourcecheck()
{
	int size;
	char *buf,*ptr;
	char w1[256],w2[256],src[256],dst[256];
	FILELIST *entry;

	snprintf(src,sizeof(src),"%sdata\\resnametable.txt",data_dir);
	buf=(char*)grfio_reads(src,size);

	if(buf==NULL) return;
	
	buf[size] = 0;

	for(ptr=buf;ptr-buf<size;) {
		if(sscanf((char*)ptr,"%[^#]#%[^#]#",w1,w2)==2){
			if(strstr(w2,"bmp")){
				snprintf(src,sizeof(src),"%sdata\\texture\\%s",data_dir,w1);
				snprintf(dst,sizeof(dst),"%sdata\\texture\\%s",data_dir,w2);
			} else {
				snprintf(src,sizeof(src),"%sdata\\%s",data_dir,w1);
				snprintf(dst,sizeof(dst),"%sdata\\%s",data_dir,w2);
			}
			entry = filelist_find(dst);
			if (entry!=NULL) {
				FILELIST fentry;
				memcpy( &fentry, entry, sizeof(FILELIST) );
				safestrcpy( fentry.fn ,src, sizeof(fentry.fn)-1 );
				filelist_modify(&fentry);
			} else {
				//ShowError("file not found in data.grf : %s < %s\n",dst,src);
			}
		}
		ptr = strchr(ptr,'\n');	// Next line
		if (!ptr) break;
		ptr++;
	}
	delete[] buf;
	filelist_adjust();	// Unnecessary area release of filelist
}

/*==========================================
 * Grfio : Resource add
 *------------------------------------------
 */
#define	GENTRY_ADDS	16	// The number increment of gentry_table entries

int grfio_add(const char *fname)
{
	int len,result;
	char *buf;

	if (gentry_entrys>=GENTRY_LIMIT) {
		ShowFatalError("gentrys limit : grfio_add\n");
		exit(1);
	}

	ShowStatus("Reading GRF File: '%s'.\n",fname);

	if (gentry_entrys>=gentry_maxentry)
	{
		const size_t sz = gentry_maxentry+GENTRY_ADDS;

		char** tmp = new char*[sz];
		if(gentry_table)
		{
			memcpy(tmp, gentry_table, gentry_maxentry*sizeof(char*));
			delete[] gentry_table;
		}
		memset(tmp+gentry_maxentry, 0, GENTRY_ADDS*sizeof(char*));
		gentry_table = tmp;
		gentry_maxentry = sz;
	}
	len = 1+strlen( fname );
	buf = new char[len];
	memcpy(buf, fname, len);
	gentry_table[gentry_entrys++] = buf;

	result = grfio_entryread(fname,gentry_entrys-1);

	if (result==0)	// Resource check
		grfio_resourcecheck();

	return result;
}

/*==========================================
 * Grfio : Finalize
 *------------------------------------------
 */
void grfio_final(void)
{
	size_t lop;

	if (filelist!=NULL)
	{
		delete[] filelist;
		filelist = NULL;
	}
	filelist_entrys = filelist_maxentry = 0;

	if (gentry_table!=NULL)
	{
		for(lop=0;lop<gentry_entrys;++lop)
		{
			if (gentry_table[lop]!=NULL)
			{
				delete[] gentry_table[lop];
			}
		}
		delete[] gentry_table;
		gentry_table = NULL;
	}
	gentry_entrys = gentry_maxentry = 0;
}

/*==========================================
 * Grfio : Initialize
 *------------------------------------------
 */
void grfio_init(const char *fname)
{
	FILE *data_conf;
	char line[1024], w1[1024], w2[1024];
	int result = 0;

	data_conf = basics::safefopen(fname, "r");

	hashinit();	// hash table initialization

	filelist = NULL;
	filelist_entrys = filelist_maxentry = 0;
	gentry_table = NULL;
	gentry_entrys = gentry_maxentry = 0;


	// It will read, if there is grf-files.txt.
	if (data_conf)
	{
		while(fgets(line, sizeof(line), data_conf))
		{
			if( !prepare_line(line) )
				continue;
			if( sscanf(line, "%1024[^:]:%1024[^\r\n]", w1, w2) == 2 ||
				sscanf(line, "%1024[^:]=%1024[^\r\n]", w1, w2) == 2 )
			{
				basics::trim(w1);
				basics::trim(w2);
				// Entry table reading
				if( strcmp(w1, "grf") == 0 ||
					strcmp(w1, "data") == 0 ||	// Primary data file
					strcmp(w1, "sdata") == 0 ||	// Sakray data file
					strcmp(w1, "adata") == 0)	// Alpha version data file
					// increment if successfully loaded
					result += (grfio_add(w2) == 0);
				else if(strcmp(w1,"data_dir") == 0)	// Data directory
					strcpy(data_dir, w2);
			}
		}
		fclose(data_conf);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",fname);

	} // end of reading grf-files.txt

	if( !result )
	{
		ShowInfo("No grf's loaded.. using default data directory\n");
	}
}
