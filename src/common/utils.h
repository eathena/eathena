// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UTILS_H_
#define _UTILS_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif
#include <stdarg.h>

// Function that dumps the hex of the first num bytes of the buffer to the screen
//#define UTIL_DUMP
#ifdef UTIL_DUMP
void dump(const unsigned char* buffer, int num);
#endif

struct StringBuf
{
	char *buf_;
	char *ptr_;
	unsigned int max_;
};

struct StringBuf* StringBuf_Malloc(void);
void StringBuf_Init(struct StringBuf* self);
int StringBuf_Printf(struct StringBuf* self, const char* fmt, ...);
int StringBuf_Vprintf(struct StringBuf* self, const char* fmt, va_list args);
int StringBuf_Append(struct StringBuf* self, const struct StringBuf *sbuf);
int StringBuf_AppendStr(struct StringBuf* self, const char* str);
int StringBuf_Length(struct StringBuf* self);
char* StringBuf_Value(struct StringBuf* self);
void StringBuf_Clear(struct StringBuf* self);
void StringBuf_Destroy(struct StringBuf* self);
void StringBuf_Free(struct StringBuf* self);

void findfile(const char *p, const char *pat, void (func)(const char*));

//Caps values to min/max
#define cap_value(a, min, max) ((a >= max) ? max : (a <= min) ? min : a)

//////////////////////////////////////////////////////////////////////////
// byte word dword access [Shinomori]
//////////////////////////////////////////////////////////////////////////

extern uint8 GetByte(uint32 val, int idx);
extern uint16 GetWord(uint32 val, int idx);
extern uint16 MakeWord(uint8 byte0, uint8 byte1);
extern uint32 MakeDWord(uint16 word0, uint16 word1);

#endif /* _UTILS_H_ */
