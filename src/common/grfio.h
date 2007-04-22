// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_GRFIO_H_
#define	_GRFIO_H_

#include "basetypes.h"

void grfio_init(const char*);					// GRFIO Initialize
void grfio_final(void);
int grfio_add(const char*);						// GRFIO Resource file add
unsigned char* grfio_read(const char*);			// GRFIO data file read
unsigned char* grfio_read(const char*, int&);	// GRFIO data file read & size get
int grfio_size(const char*);					// GRFIO data file size get

int decode_zip(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen);
int encode_zip(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen);
int decode_file (FILE *source, FILE *dest);


#endif	// _GRFIO_H_
