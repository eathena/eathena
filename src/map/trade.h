// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_TRADE_H_
#define	_TRADE_H_

#include "map.h"
void trade_traderequest(map_session_data &sd,uint32 target_id);
void trade_tradeack(map_session_data &sd,int type);
void trade_tradeadditem(map_session_data &sd,unsigned short index,uint32 amount);
void trade_tradeok(map_session_data &sd);
void trade_tradecancel(map_session_data &sd);
void trade_tradecommit(map_session_data &sd);

#endif	// _TRADE_H_
