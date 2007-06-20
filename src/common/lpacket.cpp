// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lpacket.h"


///////////////////////////////////////////////////////////////////////////////
namespace NSocket {
///////////////////////////////////////////////////////////////////////////////


/// Login failed.
CPacket006A::CPacket006A(uint8 err, const char* msg)
:	CFixOffPacket<23>()
{
	// handler,offset,-,-
	this->pid.Init(&this->_h, 0, 0, 0);
	this->err.Init(&this->_h, 2, 0, 0);
	this->msg.Init(&this->_h, 3, 0, 0);

	this->pid = 0x6a;
	this->err = err;
	if( msg != NULL )
		this->msg = msg;
}



/// Rejected from server
CPacket0074::CPacket0074(uint8 err)
:	CFixOffPacket<3>()
{
	// handler,offset,-,-
	this->pid.Init(&this->_h, 0, 0, 0);
	this->err.Init(&this->_h, 2, 0, 0);

	this->pid = 0x74;
	this->err = err;
}



/// Kick/ban notification
CPacket0081::CPacket0081(uint8 err)
:	CFixOffPacket<3>()
{
	// handler,offset,-,-
	this->pid.Init(&this->_h, 0, 0, 0);
	this->err.Init(&this->_h, 2, 0, 0);

	this->pid = 0x81;
	this->err = err;
}



/// No packet encryption
CPacket01C7::CPacket01C7()
:	CFixOffPacket<2>()
{
	// handler,offset,-,-
	this->pid.Init(&this->_h, 0, 0, 0);

	this->pid = 0x1c7;
}



/// Won event prize
CPacket023D::CPacket023D(uint32 unk)
:	CFixOffPacket<6>()
{
	// handler,offset,-,-
	this->pid.Init(&this->_h, 0, 0, 0);
	this->unk.Init(&this->_h, 2, 0, 0);

	this->pid = 0x23d;
	this->unk = unk;
}


///////////////////////////////////////////////////////////////////////////////
}// end namespace NSocket
///////////////////////////////////////////////////////////////////////////////
