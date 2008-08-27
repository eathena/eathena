// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/strlib.h"
#include "int_registry.h"
#include <stdio.h>
#include <string.h>


/// Serializes regs into the provided buffer.
/// Returns the number of bytes written.
int inter_regs_tobuf(uint8* buf, size_t size, const struct regs* reg)
{
	int c = 0;
	int i;

	for( i = 0; i < reg->reg_num; i++)
	{
		c += sprintf((char*)(buf+c), "%s", reg->reg[i].str)+1; //We add 1 to consider the '\0' in place.
		c += sprintf((char*)(buf+c), "%s", reg->reg[i].value)+1;
	}

	return c;
}

/// Retrieves regs from the provided buffer.
/// Returns ??
int inter_regs_frombuf(const uint8* buf, size_t size, struct regs* reg)
{
	size_t p; // cursor
	int j;

	//FIXME: doesn't watch the buffer size properly
	//FIXME: doesn't respect the specific regs maxima
	for( j = 0, p = 0; j < GLOBAL_REG_NUM && p < size; ++j )
	{
		safestrncpy(reg->reg[j].str, (const char*)(buf+p), sizeof(reg->reg[j].str));
		p += strlen(reg->reg[j].str)+1; //+1 to skip the '\0' between strings.
		safestrncpy(reg->reg[j].value, (const char*)(buf+p), sizeof(reg->reg[j].value));
		p += strlen(reg->reg[j].value)+1;
	}

	reg->reg_num = j;

	return 0;
}
