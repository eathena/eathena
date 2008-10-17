// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STATUS_H_
#define _INT_STATUS_H_

#include "../common/cbasetypes.h" // uint8
#include "statusdb.h" // struct scdata

void inter_status_init(void);
void inter_status_final(void);
void inter_status_sync(void);
bool inter_status_delete(int char_id);
int inter_status_tobuf(uint8* buf, size_t size, const struct scdata* sc);
bool inter_status_frombuf(uint8* buf, size_t size, struct scdata* sc);
int inter_status_parse_frommap(int fd);

#endif /* _INT_STATUS_H_ */
