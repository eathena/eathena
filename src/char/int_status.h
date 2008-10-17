// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STATUS_H_
#define _INT_STATUS_H_

void inter_status_init(void);
void inter_status_final(void);
void inter_status_sync(void);
bool inter_status_delete(int char_id);
int inter_status_parse_frommap(int fd);

#endif /* _INT_STATUS_H_ */
