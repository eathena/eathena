// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LPACKET_H_
#define _LPACKET_H_

#include "basepacket.h"


///////////////////////////////////////////////////////////////////////////////
namespace NSocket {
///////////////////////////////////////////////////////////////////////////////
// The client has two modes:
// - login mode (login+char server)
// - game mode (map server)
// Different packets are supported during each mode.
// This file contains packets that are only available during the login mode.
//
// Implemented:
// Server->Client : 006A,0074,0081,01C7,023D
// Client->Server : 
//
//## TODO
// implement:
// - Server->Client : 0069,006B,006C,006D,006E,006F,0070,0071,0073,01BE,01F1,020D,028B,028E,0290
// - Client->Server : 0064,0065,0066,0067,0068,009B,0187,01BF,01DB,01DD,01FA,01FB,0200,0204,0277,027C,028F,...(?)
// check which packets are also available in game mode


///////////////////////////////////////////////////////////////////////////////
// Fields
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Packets
///////////////////////////////////////////////////////////////////////////////





/// S->C
/// Login failed (disconnects and goes to login screen)
///
/// err:
/// 0 = MsgStringTable[6] = "Unregistered ID"
/// 1 = MsgStringTable[267] = "Incorrect Password"
/// 2 = MsgStringTable[8] = "This ID is expired"
/// 4 = MsgStringTable[266] = "You have been blocked by the GM Team"
/// 5 = MsgStringTable[310] = "Your Game's EXE file is not the latest version"
/// 6 = MsgStringTable[449] = "Your are Prohibited to log in until %s"
/// 7 = MsgStringTable[439] = "Server is jammed due to over populated"
/// 8 = MsgStringTable[681] = "No more accounts may be connected from this company"
/// 9 = MsgStringTable[703] = "MSI_REFUSE_BAN_BY_DBA"
/// 10 = MsgStringTable[704] = "MSI_REFUSE_EMAIL_NOT_CONFIRMED"
/// 11 = MsgStringTable[705] = "MSI_REFUSE_BAN_BY_GM"
/// 12 = MsgStringTable[706] = "MSI_REFUSE_TEMP_BAN_FOR_DBWORK"
/// 13 = MsgStringTable[707] = "MSI_REFUSE_SELF_LOCK"
/// 14 = MsgStringTable[708] = "MSI_REFUSE_NOT_PERMITTED_GROUP"
/// 15 = MsgStringTable[709] = "MSI_REFUSE_WAIT_FOR_SAKRAY_ACTIVE"
/// 16 = if servicetype korea -> opens browser in page to change password
/// 99 = MsgStringTable[368] = "This ID has been totally erased"
/// 100 = MsgStringTable[809] = "Login information remains at %s"
/// 101 = MsgStringTable[810] = "Account has been locked for a hacking investigation"
/// 102 = MsgStringTable[811] = "This account has been temporarily prohibited from login due to a bug-related investigation"
/// 103 = MsgStringTable[859] = "This character is being deleted"
/// 104 = MsgStringTable[860] = "Your spouse character is being deleted"
/// other = MsgStringTable[9] = "Rejected from server"
///
/// msg: used in err=6, err=100 and err=101
class CPacket006A : public CFixPacket<3,23>
{
public:
	CFieldW				pid;// packet id
	CFieldB				err;// error code
	CFieldFixString<20>	msg;// error message
public:
	CPacket006A(uint8 err, const char* msg=NULL);
};



/// S->C
/// Rejected from server (disconnects and goes to login screen)
///
/// MsgStringTable[9] = "Rejected from server"
class CPacket0074 : public CFixPacket<2,3>
{
public:
	CFieldW	pid;// packet id
	CFieldB	err;// error code
public:
	CPacket0074(uint8 err=0);
};



/// S->C
/// Kick/ban notification (disconnects and goes to login screen)
///
/// err:
/// 1 = MsgStringTable[4] = "Server connection closed"
/// 2 = MsgStringTable[5] = "Someone has already logged in with this ID"
/// 3 = MsgStringTable[241] = "Too much lag" (?)
/// 4 = MsgStringTable[264] = "Server is jammed due to over population. Try again shortly."
/// 5 = MsgStringTable[305] = "You are underaged"
/// 6 = MsgStringTable[378] = "You didn't pay for this ID. Would you like to pay for it now?"
///     if yes is selected, opens join/payment browser page:
///        servicetype america/china - hardcoded address
///        servicetype japan - doesn't open browser
///        other - address is MsgStringTable[528]
/// 7 = MsgStringTable[439] = "Server is jammed due to over population. Try again after a few minutes."
/// 8 = MsgStringTable[440] = "Server still recognizes your last log-in"
/// 9 = MsgStringTable[529] = "IP capacity of this Internet Cafe is full"
/// 10 = MsgStringTable[530] = "Payed game time is over"
/// 11 = MsgStringTable[575] = "This account is reserved"
/// 12 = MsgStringTable[576] = "Rating system glitch"
/// 13 = MsgStringTable[577] = "IP conflict"
/// 14 = MsgStringTable[578] = "" (?)
/// 16 = MsgStringTable[606] = "" (?)
/// 17 = MsgStringTable[607] = "Your game ticket has expired" (?)
/// 18 = MsgStringTable[678] = "A character is already connected on this account"
/// 100 = MsgStringTable[1123] = "" (?)
/// 101 = MsgStringTable[1178] = "More than 30 players sharing the same IP have logged into the game for an hour"
/// 102 = MsgStringTable[1179] = "More than 10 connections sharing the same IP have logged into the game for an hour"
/// other = MsgStringTable[3] = "Disconnected"
class CPacket0081 : public CFixPacket<2,3>
{
public:
	CFieldW	pid;// packet id
	CFieldB	err;// error code
public:
	CPacket0081(uint8 err=0);
};



/// S->C
/// Error message "No Packet Encryption !!!"
class CPacket01C7 : public CFixPacket<1,2>
{
public:
	CFieldW	pid;// packet id
public:
	CPacket01C7();
};



/// Displays message MsgStringTable[1020] = "[EVENT] You have won an event prize. Please claim your prize in game."
class CPacket023D : public CFixPacket<2,6>
{
public:
	CFieldW	pid;// packet id
	CFieldL unk;// unknown
public:
	CPacket023D(uint32 unk=0);
};


///////////////////////////////////////////////////////////////////////////////
}// end namespace NSocket
///////////////////////////////////////////////////////////////////////////////

#endif//_LPACKET_H_
