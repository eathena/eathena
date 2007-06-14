// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lpacket.h"


///////////////////////////////////////////////////////////////////////////////
namespace NSocket {
///////////////////////////////////////////////////////////////////////////////


/// Login failed.
CPacket006A::CPacket006A(uint8 err, const char* msg)
:	CFixPacket<3,23>()
{
	NFieldHandler::CIdxAutoRegistry reg(this->_h);
	reg << this->pid;
	reg << this->err;
	reg << this->msg;

	this->pid = 0x6a;
	this->err = err;
	if( msg != NULL )
		this->msg = msg;
}



/// Rejected from server
CPacket0074::CPacket0074(uint8 err)
:	CFixPacket<2,3>()
{
	NFieldHandler::CIdxAutoRegistry reg(this->_h);
	reg << this->pid;
	reg << this->err;

	this->pid = 0x74;
	this->err = err;
}



/// Kick/ban notification
CPacket0081::CPacket0081(uint8 err)
:	CFixPacket<2,3>()
{
	NFieldHandler::CIdxAutoRegistry reg(this->_h);
	reg << this->pid;
	reg << this->err;

	this->pid = 0x81;
	this->err = err;
}



/// No packet encryption
CPacket01C7::CPacket01C7()
:	CFixPacket<1,2>()
{
	NFieldHandler::CIdxAutoRegistry reg(this->_h);
	reg << this->pid;

	this->pid = 0x1c7;
}



/// Won event prize
CPacket023D::CPacket023D(uint32 unk)
:	CFixPacket<2,6>()
{
	NFieldHandler::CIdxAutoRegistry reg(this->_h);
	reg << this->pid;
	reg << this->unk;

	this->pid = 0x23d;
	this->unk = unk;
}


///////////////////////////////////////////////////////////////////////////////
}// end namespace NSocket
///////////////////////////////////////////////////////////////////////////////
