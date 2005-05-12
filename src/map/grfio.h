// $Id: grfio.h,v 1.1.1.1 2004/09/10 17:44:49 MagicalTux Exp $
#ifndef	_GRFIO_H_
#define	_GRFIO_H_

void grfio_init(char*);			// GRFIO Initialize
void grfio_final(void);			// GRFIO Finalize
int grfio_add(char*);			// GRFIO Resource file add
void* grfio_read(char*);		// GRFIO data file read
void* grfio_reads(char*,int*);	// GRFIO data file read & size get
int grfio_size(char*);			// GRFIO data file size get

int decode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
int encode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
int decode_file (FILE *source, FILE *dest);

#endif	// _GRFIO_H_
