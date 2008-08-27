// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_REGISTRY_H_
#define _INT_REGISTRY_H_

#include "../common/cbasetypes.h"
#include "../common/mmo.h"

int inter_regs_tobuf(uint8* buf, size_t size, const struct regs* reg);
int inter_regs_frombuf(const uint8* buf, size_t size, struct regs* reg);

int inter_accreg_init(void);
int inter_accreg_final(void);
bool inter_accreg_load(int account_id, struct regs* reg);
bool inter_accreg_save(int account_id, struct regs* reg);

int inter_charreg_init(void);
int inter_charreg_final(void);
bool inter_charreg_load(int char_id, struct regs* reg);
bool inter_charreg_save(int char_id, struct regs* reg);

#ifdef TXT_ONLY
int inter_accreg_sync(void);
int inter_charreg_tostr(char* str, const struct regs* reg);
bool inter_charreg_fromstr(const char* str, struct regs* reg);
#endif

#endif /* _INT_REGISTRY_H_ */
