#ifndef _PACKETDB_H_
#define _PACKETDB_H_

#include "basetypes.h"

struct map_session_data;

// packet DB
#define MAX_PACKET_DB		0x02af
#define MAX_PACKET_VER		20


class packet_cmd
{
public:
	short len;
	int (*func)(int,map_session_data &);
	short pos[20];

	packet_cmd()	{}
	~packet_cmd()	{}
	packet_cmd(short l, int (*f)(int,map_session_data &)=NULL, short p00=0, short p01=0, short p02=0, short p03=0, short p04=0, short p05=0, short p06=0, short p07=0, short p08=0, short p09=0, short p10=0, short p11=-11, short p12=0, short p13=0, short p14=0, short p15=0, short p16=0, short p17=0, short p18=0, short p19=0)
		: len(l),
		  func(f)
	{
		pos[0]=p00;
		pos[1]=p01;
		pos[2]=p02;
		pos[3]=p03;
		pos[4]=p04;
		pos[5]=p05;
		pos[6]=p06;
		pos[7]=p07;
		pos[8]=p08;
		pos[9]=p09;
		pos[10]=p10;
		pos[11]=p11;
		pos[12]=p12;
		pos[13]=p13;
		pos[14]=p14;
		pos[15]=p15;
		pos[16]=p16;
		pos[17]=p17;
		pos[18]=p18;
		pos[19]=p19;
	}
};

const packet_cmd& packet(uint packet_ver, uint cmd);
uint packet_connect(uint packet_ver);
int packetdb_readdb(void);


#endif//_PACKETDB_H_

