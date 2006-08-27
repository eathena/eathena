// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_VENDING_H_
#define	_VENDING_H_

#include "map.h"

void vending_closevending(struct map_session_data &sd);
void vending_openvending(struct map_session_data &sd,unsigned short len,const char *message,int flag, unsigned char *buffer);
void vending_vendinglistreq(struct map_session_data &sd,uint32 id);
void vending_purchasereq(struct map_session_data &sd,unsigned short len,uint32 id, unsigned char *buffer);

#endif	// _VENDING_H_
