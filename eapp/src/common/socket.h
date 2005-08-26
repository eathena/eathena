// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_SOCKET_H_
#define _SOCKET_H_

#include "base.h"
#include "malloc.h"
#include "timer.h"


extern time_t last_tick;


///////////////////////////////////////////////////////////////////////////////
// IP number stuff
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// virtual interface for ip numbers and operations
///////////////////////////////////////////////////////////////////////////////
class ipaddr
{
public:
	ipaddr()			{}
	virtual ~ipaddr()	{}
	///////////////////////////////////////////////////////////////////////////
	virtual const ulong addr() const { return INADDR_LOOPBACK; }
	virtual ulong& addr() { static ulong dummy; dummy=INADDR_LOOPBACK; return dummy; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ulong mask() const { return INADDR_BROADCAST; }
	virtual ulong& mask() { static ulong dummy; dummy=INADDR_BROADCAST; return dummy; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return 0; }
	virtual ushort& port() { static ushort dummy; dummy=0; return dummy; }
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	virtual const char *getstring(char *buffer=NULL) = 0;
	///////////////////////////////////////////////////////////////////////////
};

///////////////////////////////////////////////////////////////////////////////
// class for ip numbers and helpers
///////////////////////////////////////////////////////////////////////////////
class ipaddress : public ipaddr
{
private:
	///////////////////////////////////////////////////////////////////////////
	// class helper
	// does network initialisation and gets available system ips
	template <uint C> class _ipset_helper
	{	
		ulong	cAddr[C];	// ip addresses of local host (host byte order)
		uint	cCnt;		// # of ip addresses

	public:
		_ipset_helper() : cCnt(0)
		{
#ifdef WIN32
			uchar** a;
			unsigned int i;
			char fullhost[255];
			struct hostent* hent;
			
			/* Start up the windows networking */
			WSADATA wsaData;
			if ( WSAStartup(WINSOCK_VERSION, &wsaData) != 0 )
			{
				printf("SYSERR: WinSock not available!\n");
				exit(1);
			}
			
			if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR)
			{
				printf("No hostname defined!\n");
				return;
			} 
			
			// XXX This should look up the local IP addresses in the registry
			// instead of calling gethostbyname. However, the way IP addresses
			// are stored in the registry is annoyingly complex, so I'll leave
			// this as T.B.D.
			hent = gethostbyname(fullhost);
			if (hent == NULL) {
				printf("Cannot resolve our own hostname to a IP address");
				return;
			}
			a = (uchar**)hent->h_addr_list;
			for(i = 0; a[i] != NULL && i < C; ++i) {
				cAddr[i] =	  (a[i][0]<<0x18)
							| (a[i][1]<<0x10)
							| (a[i][2]<<0x08)
							| (a[i][3]);
			}
			cCnt = i;
#else//not W32
			int pos;
			int fdes = socket(AF_INET, SOCK_STREAM, 0);
			char buf[16 * sizeof(struct ifreq)];
			struct ifconf ic;
			
			// The ioctl call will fail with Invalid Argument if there are more
			// interfaces than will fit in the buffer
			ic.ifc_len = sizeof(buf);
			ic.ifc_buf = buf;
			if(ioctl(fdes, SIOCGIFCONF, &ic) == -1) {
				printf("SIOCGIFCONF failed!\n");
				return;
			}
			for(pos = 0; pos < ic.ifc_len;   )
			{
				struct ifreq * ir = (struct ifreq *) (ic.ifc_buf + pos);
				struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);
				
				if(a->sin_family == AF_INET) {
					u_long ad = ntohl(a->sin_addr.s_addr);
					if(ad != INADDR_LOOPBACK) {
						cAddr[cCnt++] = ad;
						if(cCnt == C)
							break;
					}
				}
#if defined(_AIX) || defined(__APPLE__)
				pos += ir->ifr_addr.sa_len;
				pos += sizeof(ir->ifr_name);
#else// not AIX or APPLE
				pos += sizeof(struct ifreq);
#endif//not AIX or APPLE
			}
#endif//not W32	
		}
		///////////////////////////////////////////////////////////////////////
		// number of found system ip's (w/o localhost)
		uint GetSystemIPCount()	const
		{
			return cCnt;
		}
		///////////////////////////////////////////////////////////////////////
		// get an address from the array, return loopback on error
		ipaddress GetSystemIP(uint i=0) const
		{
			if( i < cCnt )
				return cAddr[i];
			else if(cCnt>0)
				return cAddr[0];
			return INADDR_LOOPBACK;
		}
	};
	///////////////////////////////////////////////////////////////////////////
	// need a singleton, this here is safe, 
	// we need it only once and destruction order is irrelevant
	static _ipset_helper<16>& gethelper()
	{
		static _ipset_helper<16> iphelp;
		return iphelp;
	}
	
public:
	static ipaddress GetSystemIP(uint i=0) { return gethelper().GetSystemIP(i); }
	static uint GetSystemIPCount()	{ return gethelper().GetSystemIPCount(); }
	static bool isBindable(ipaddress ip)
	{	// check if an given IP is part of the system IP that can be bound to
		for(uint i=0; i<GetSystemIPCount(); i++)
			if( ip==GetSystemIP(i) )
				return true;
		return false;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// class data
    union
    {
        uchar   bdata[4];
        ulong   ldata;
    };
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor/destructor
    ipaddress():ldata(INADDR_ANY)	{}
	~ipaddress()					{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    ipaddress(const ipaddress& a) : ldata(a.ldata)	{}
    ipaddress& operator= (const ipaddress& a)	{ this->ldata = a.ldata; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set (needs explicite casts when initializing with 0)
    ipaddress(ulong a):ldata(a)	{}
	ipaddress(const char* str):ldata(str2ip(str))	{}
    ipaddress(int a, int b, int c, int d)
	{
		ldata =	 (a&0xFF) << 0x18
				|(b&0xFF) << 0x10
				|(c&0xFF) << 0x08
				|(d&0xFF);
	}
	///////////////////////////////////////////////////////////////////////////
	// assignment set (needs explicite casts when assigning 0)
    ipaddress& operator= (ulong a)			{ ldata = a; return *this; }
	ipaddress& operator= (const char* str)	{ ldata = str2ip(str); return *this; }
	bool init(const char *str)
	{
		ldata = str2ip(str);
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// bytewise access (writable and readonly)
    uchar& operator [] (int i)                  
	{ 
#ifdef CHECK_BOUNDS
		if( (i>=4) || (i<0) )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("ipaddress: out of bound access");
#else//!CHECK_EXCEPTIONS
			static uchar dummy;
			return dummy=0;
#endif//!CHECK_EXCEPTIONS
		}
#endif//CHECK_BOUNDS

		if( MSB_FIRST == CheckByteOrder() )
			return bdata[3-i];
		else
			return bdata[i];
	}
    const uchar operator [] (int i) const
	{ 
#ifdef CHECK_BOUNDS
		if( (i>=4) || (i<0) )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("ipaddress: out of bound access");
#else//!CHECK_EXCEPTIONS
			return 0;
#endif//!CHECK_EXCEPTIONS
		}
#endif//CHECK_BOUNDS

		if( MSB_FIRST == CheckByteOrder() )
			return bdata[3-i];
		else
			return bdata[i];
	}

	///////////////////////////////////////////////////////////////////////////
	// pod access on the ip (host byte order)
    operator ulong() const	{ return ldata; }

	///////////////////////////////////////////////////////////////////////////
	// virtual access interface
	virtual const ulong addr() const { return ldata; }
	virtual ulong& addr() { return ldata; }
	///////////////////////////////////////////////////////////////////////////
	// ip2string
	virtual const char *getstring(char *buffer=NULL)
	{	// usage of the static buffer is not threadsafe
		static char tmp[16];
		char *buf = (buffer) ? buffer : tmp;
		sprintf(buf, "%d.%d.%d.%d",bdata[3],bdata[2],bdata[1],bdata[0]);
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////
	// converts a string to an ip (host byte order)
	static ipaddress str2ip(const char *str)
	{	// format: <ip>
		struct hostent*h;
		while( isspace( ((unsigned char)(*str)) ) ) str++;
		h = gethostbyname(str);
		if (h != NULL)
			return ipaddress( MakeDWord((unsigned char)h->h_addr[3], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[0]) );
		else
			return ipaddress( ntohl(inet_addr(str)) );
	}
	static bool str2ip(const char* str, ipaddress &addr, ipaddress &mask, ushort &port)
	{	// format: <ip>/<mask>:<port>
		bool ret = false;
		if(str)
		{
			char buffer[1024];
			char *kp=NULL, *mp=NULL;
			kp = strchr(str,'/'); // the first ip/mask seperator
			mp=strchr(str,':'); // the second ip/port seperator
			if(kp && mp)
			{	// all data given
				// <ip>
				memcpy(buffer, str, kp-str);
				buffer[kp-str]=0;
				addr = ipaddress::str2ip(buffer);
				// <mask>
				if(kp<mp)
				{	// format is ok
					kp++;
					memcpy(buffer, kp, mp-kp);
					buffer[mp-kp]=0;
					mask = ipaddress::str2ip(buffer);
				}
				else
				{	// the mask seperator placement is wrong
					mask = INADDR_BROADCAST;
				}
				// <port>
				port = atoi(mp+1);
			}
			else if(!kp && mp)
			{	// mo mask given
				// <ip>
				memcpy(buffer, str, mp-str);
				buffer[mp-str]=0;
				addr = ipaddress::str2ip(buffer);
				// default mask
				mask = INADDR_BROADCAST; // 255.255.255.255
				// <port>
				port = atoi(mp+1);
			}
			else if(kp && !mp)
			{	// no port given
				// <ip>
				memcpy(buffer, str, kp-str);
				buffer[kp-str]=0;
				addr = ipaddress::str2ip(buffer);
				// <mask>
				kp++;
				memcpy(buffer, kp, mp-kp);
				buffer[mp-kp]=0;
				mask = ipaddress::str2ip(buffer);
				// don't change the port
			}
			else
			{	// neither mask nor port given
				// <ip>
				addr = ipaddress::str2ip(str);
				// default mask
				mask = INADDR_BROADCAST; // 255.255.255.255
				// don't change the port
			}
			ret = true;
		}
		return ret;
	}
};

///////////////////////////////////////////////////////////////////////////////
//
// class for a network address (compound of an ip address and a port number)
//
///////////////////////////////////////////////////////////////////////////////
class netaddress : public ipaddr
{
	friend class ipset;
protected:
	///////////////////////////////////////////////////////////////////////////
	// class data
	ipaddress	cAddr;
	ushort		cPort;
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor/destructor
    netaddress():cAddr((ulong)INADDR_ANY),cPort(0)	{}
	~netaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    netaddress(const netaddress& a):cAddr(a.cAddr),cPort(a.cPort){}
    netaddress& operator= (const netaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set
	netaddress(ulong a, ushort p):cAddr(a),cPort(p)	{}
	netaddress(ushort p):cAddr((ulong)INADDR_ANY),cPort(p)	{}
    netaddress(int a, int b, int c, int d, ushort p):cAddr(a,b,c,d),cPort(p) {}
	netaddress(const char* str)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	// assignment set
    netaddress& operator= (ulong a)			{ this->cAddr = a; return *this; }
	netaddress& operator= (ushort p)		{ this->cPort = p; return *this; }
	netaddress& operator= (const char* str)	{ init(str); return *this; }
	bool init(const char *str)
	{	
		ipaddress mask; // dummy
		return ipaddress::str2ip(str, this->cAddr, mask, this->cPort);
	}

	///////////////////////////////////////////////////////////////////////////
	// virtual access interface
	virtual const ulong addr() const { return cAddr.ldata; }
	virtual ulong& addr() { return cAddr.ldata; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return cPort; }
	virtual ushort& port()		 { return cPort; }
	///////////////////////////////////////////////////////////////////////////
	// networkaddr2string
	virtual const char *getstring(char *buffer=NULL)
	{	// usage of the static buffer is not threadsafe
		static char tmp[32];
		char *buf = (buffer) ? buffer : tmp;
		sprintf(buf, "%d.%d.%d.%d:%d",
			cAddr[3],cAddr[2],cAddr[1],cAddr[0],
			cPort);
		return buf;
	}
	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const netaddress s) const { return cAddr==s.cAddr && cPort==s.cPort; }
	bool operator != (const netaddress s) const { return cAddr!=s.cAddr || cPort!=s.cPort; }

};
///////////////////////////////////////////////////////////////////////////////
//
// class for a subnetwork address (ip address, subnetmask and port number)
//
///////////////////////////////////////////////////////////////////////////////
class subnetaddress : public netaddress
{
	friend class ipset;
protected:
	///////////////////////////////////////////////////////////////////////////
	// class data
	ipaddress cMask;
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor/destructor
    subnetaddress():cMask(INADDR_BROADCAST)	{}
	~subnetaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    subnetaddress(const subnetaddress& a):netaddress(a),cMask(a.cMask) {}
    subnetaddress& operator= (const subnetaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; this->cMask=a.cMask; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set
	subnetaddress(ulong a, ulong m, ushort p):netaddress(a,p),cMask(m)	{}
	subnetaddress(netaddress a, ipaddress m):netaddress(a),cMask(m)	{}
	subnetaddress(const char* str)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	// assignment set
	subnetaddress& operator= (const char* str)	{ init(str); return *this; }
	bool init(const char *str)
	{	// format: <ip>/<mask>:<port>
		return ipaddress::str2ip(str, this->cAddr, this->cMask, this->cPort);
	}
	///////////////////////////////////////////////////////////////////////////
	// virtual access interface
	virtual const ulong mask() const { return cMask.ldata; }
	virtual ulong& mask() { return cMask.ldata; }
	///////////////////////////////////////////////////////////////////////////
	// networkaddr2string
	virtual const char *getstring(char *buffer=NULL)
	{	// usage of the static buffer is not threadsafe
		static char tmp[64];
		char *buf = (buffer) ? buffer : tmp;
		sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
			this->cAddr[3],this->cAddr[2],this->cAddr[1],this->cAddr[0], 
			this->cMask[3],this->cMask[2],this->cMask[1],this->cMask[0], 
			this->cPort);
		return buf;
	}
	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const subnetaddress s) const { return cMask==s.cMask && cAddr==s.cAddr && cPort==s.cPort; }
	bool operator != (const subnetaddress s) const { return cMask!=s.cMask || cAddr!=s.cAddr || cPort!=s.cPort; }

};


///////////////////////////////////////////////////////////////////////////////
// class for IP-sets 
// stores wan/lan ips with lan subnet and wan/lan ports
// can automatically fill default values
///////////////////////////////////////////////////////////////////////////////
class ipset : public ipaddr
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	subnetaddress	lanaddr;
	netaddress		wanaddr;
	
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	// init with 0, need to
	ipset(const ipaddress lip = ipaddress::GetSystemIP(0),	// identify with the first System IP by default
		  const ipaddress lsu = INADDR_BROADCAST,			// 255.255.255.255
		  const ushort lpt    = 0,
		  const ipaddress wip = (ulong)INADDR_ANY,			// 0.0.0.0
		  const ushort wpt    = 0 )
		: lanaddr(lip,lsu,lpt),wanaddr(wip,wpt)
	{}
	ipset(const ulong lip,
		  const ulong lsu,
		  const ushort lpt,
		  const ulong wip,
		  const ushort wpt)
		: lanaddr(lip,lsu,lpt),wanaddr(wip,wpt)
	{}

	ipset(const ushort lpt,
		  const ushort wpt)
		: lanaddr(ipaddress::GetSystemIP(0),INADDR_BROADCAST,lpt),wanaddr(INADDR_ANY,wpt)
	{}
	ipset(const ushort pt)
		: lanaddr(ipaddress::GetSystemIP(0),INADDR_BROADCAST,pt),wanaddr(INADDR_ANY,pt)
	{}

	~ipset()	{}
	// can use default copy/assign here

	///////////////////////////////////////////////////////////////////////////
	// construction set
	ipset(const char* str)	{ init(str); }

	///////////////////////////////////////////////////////////////////////////
	// assignment set
	const ipset& operator=(const char* str)	{ init(str); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// initializes from a format string
	// automatically checks the ips for local usage
	bool init(const char *str)
	{	// reading formatstring:
		// "<whitespace>*wanip:wanport,<whitespace>*lanip/lanmask:lanport.*"
		bool ret = false;
		if(str)
		{	
			char buffer[1024];
			const char *ip, *jp, *kp;
			ip = strchr(str,',');
			if(ip)
			{	// given two ip strings
				// copy the first
				if(str+sizeof(buffer) < ip) ip = str+sizeof(buffer)-1;
				memcpy(buffer,str,ip-str);
				buffer[ip-str] = 0;
				// skip the comma
				ip++; 
				// check if the first ip string has a subnet
				jp = strchr(buffer,'/');
				// check if the second ip string has a subnet
				kp = strchr(ip,'/');

				//if( (jp&&kp) || (jp&&!kp) ) ->
				if( jp )
				{	// both have subnets 
					// first has sub, second has none
					// assume order: "lan,wan"
					lanaddr = buffer;
					wanaddr = ip;
				}
				else if(!jp && kp)
				{	// first has none, second has sub
					lanaddr = ip;
					wanaddr = buffer;
				}
				else
				{	// no subnets; only take the first ip, default the second
					lanaddr = buffer;
					wanaddr = netaddress((ulong)INADDR_ANY, lanaddr.cPort);
				}
			}
			else
			{	// only one given, assume it the lanip
				lanaddr = str;
				wanaddr = netaddress((ulong)INADDR_ANY, lanaddr.cPort);
			}
		}
		checklocal();
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// checks if the ip's are locally available and correct wrong entries
	// returns true on ok, false if something has changed
	bool checklocal()
	{
		bool ret = true;
		if(lanaddr.cAddr==INADDR_ANY) // not detected
		{	// take the first system ip for wan and loopback for lan
			lanaddr.cAddr = ipaddress::GetSystemIP(0);
			ret = false;
		}
		ret &= checkPorts();
		return ret;
	}
	bool checkPorts()
	{	// check for unset wan address/port
		if( wanaddr.port() == 0 ) 
			wanaddr.port() = lanaddr.port();
		if( lanaddr.port() == 0 ) 
			lanaddr.port() = wanaddr.port();
		return (lanaddr.port() != 0) ;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// check if an given ip is LAN
	bool isLAN(const ipaddress ip) const
	{
		return ( (lanaddr.addr()&lanaddr.mask()) == (ip.addr()&lanaddr.mask()) );
	}
	///////////////////////////////////////////////////////////////////////////
	// check if an given ip is WAN
	bool isWAN(const ipaddress ip) const
	{
		return ( (lanaddr.addr()&lanaddr.mask()) != (ip.addr()&lanaddr.mask()) );
	}
	///////////////////////////////////////////////////////////////////////////
	bool SetLANIP(const ipaddress ip)
	{	// a valid lan address should be bindable
		if( ipaddress::isBindable(ip) ) 
		{
			lanaddr.addr() = ip;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress LANIP() const	{ return lanaddr.cAddr; }
	ipaddress& LANMask()	{ return lanaddr.cMask; }
	ushort& LANPort()		{ return lanaddr.cPort; }
	ipaddress& WANIP()		{ return wanaddr.cAddr; }
	ushort& WANPort()		{ return wanaddr.cPort; }

	///////////////////////////////////////////////////////////////////////////
	// returning as netaddresses only
	netaddress& LANAddr()	{ return lanaddr; }	
	netaddress& WANAddr()	{ return wanaddr; }

	///////////////////////////////////////////////////////////////////////////
	// virtual interface
	virtual const ulong addr() const { return lanaddr.cAddr; }
	virtual ulong& addr() { return lanaddr.cAddr.ldata; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ulong mask() const { return lanaddr.cMask; }
	virtual ulong& mask() { return lanaddr.cMask.ldata; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return lanaddr.cPort; }
	virtual ushort& port() { return lanaddr.cPort; }
	///////////////////////////////////////////////////////////////////////////
	virtual const char *getstring(char *buffer=NULL)
	{	// usage of the static buffer is not threadsafe
		static char tmp[64];
		char *buf = (buffer) ? buffer : tmp;
		if(lanaddr.cMask == INADDR_BROADCAST)
		{	// have only one accessable ip
			sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
				lanaddr.cAddr[3],lanaddr.cAddr[2],lanaddr.cAddr[1],lanaddr.cAddr[0], 
				lanaddr.cMask[3],lanaddr.cMask[2],lanaddr.cMask[1],lanaddr.cMask[0], 
				lanaddr.cPort);
		}
		else
		{	// have a full set
			sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d, %d.%d.%d.%d:%d",
				lanaddr.cAddr[3],lanaddr.cAddr[2],lanaddr.cAddr[1],lanaddr.cAddr[0], 
				lanaddr.cMask[3],lanaddr.cMask[2],lanaddr.cMask[1],lanaddr.cMask[0], 
				lanaddr.cPort,
				wanaddr.cAddr[3],wanaddr.cAddr[2],wanaddr.cAddr[1],wanaddr.cAddr[0], 
				wanaddr.cPort);
		}
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const ipset s) const { return lanaddr==s.lanaddr && wanaddr==s.wanaddr; }
	bool operator != (const ipset s) const { return lanaddr!=s.lanaddr || wanaddr!=s.wanaddr; }

};


///////////////////////////////////////////////////////////////////////////////
// predeclares
class streamable;
class buffer_iterator;


//////////////////////////////////////////////////////////////////////////
// char buffer access
//////////////////////////////////////////////////////////////////////////


class buffer_iterator
{
	mutable unsigned char *ipp;	// read/write pointer inside the buffer
	const unsigned char *start;	// pointer to the start of the buffer
	const unsigned char *end;	// pointer to the end of the buffer
public:
	buffer_iterator(const unsigned char* b, size_t sz)
		: ipp(const_cast<unsigned char*>(b)), start(b), end(b+sz)
	{}

	bool eof()			{ return !(ipp && ipp<end); }
	size_t size()		{ return ipp-start; }
	size_t freesize()	{ return end-ipp; }

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		if( ipp && ipp<end)
			*ipp++ = ch;
		return ch;
	}
	char operator = (const char ch)
	{	
		if( ipp && ipp<end)
			*ipp++ = (unsigned char)ch;
		return ch;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned char ()
	{	
		if( ipp && ipp<end)
			return *ipp++;
		return 0;
	}
	operator char()
	{	
		if( ipp && ipp<end)
			return *ipp++;
		return 0;

	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{	// implement little endian buffer format
		if( ipp && ipp+1<end)
		{
			*ipp++ = (unsigned char)(sr         ); 
			*ipp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	short operator = (const short sr)
	{	// implement little endian buffer format
		if( ipp && ipp+1<end)
		{
			*ipp++ = (unsigned char)(sr         ); 
			*ipp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short()
	{	// implement little endian buffer format
		unsigned short sr=0;
		if( ipp && ipp+1<end)
		{	
			sr  = ((unsigned short)(*ipp++)        ); 
			sr |= ((unsigned short)(*ipp++) << 0x08);
		}
		return sr;
	}
	operator short ()
	{	// implement little endian buffer format
		short sr=0;
		if( ipp && ipp+1<end)
		{	
			sr  = ((unsigned short)(*ipp++)        ); 
			sr |= ((unsigned short)(*ipp++) << 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{	// implement little endian buffer format
		if( ipp && ipp+3<end)
		{
			*ipp++ = (unsigned char)(ln          );
			*ipp++ = (unsigned char)(ln  >> 0x08 );
			*ipp++ = (unsigned char)(ln  >> 0x10 );
			*ipp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	long operator = (const long ln)
	{	// implement little endian buffer format
		if( ipp && ipp+3<end)
		{
			*ipp++ = (unsigned char)(ln          );
			*ipp++ = (unsigned char)(ln  >> 0x08 );
			*ipp++ = (unsigned char)(ln  >> 0x10 );
			*ipp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long ()
	{	// implement little endian buffer format
		unsigned long ln=0;
		if( ipp && ipp+3<end)
		{	
			ln  = ((unsigned long)(*ipp++)        ); 
			ln |= ((unsigned long)(*ipp++) << 0x08);
			ln |= ((unsigned long)(*ipp++) << 0x10);
			ln |= ((unsigned long)(*ipp++) << 0x18);
		}
		return ln;
	}
	operator long ()
	{	// implement little endian buffer format
		long ln=0;
		if( ipp && ipp+3<end)
		{	
			ln  = ((unsigned long)(*ipp++)        ); 
			ln |= ((unsigned long)(*ipp++) << 0x08);
			ln |= ((unsigned long)(*ipp++) << 0x10);
			ln |= ((unsigned long)(*ipp++) << 0x18);
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{	// implement little endian buffer format
		// IPs are given in network byte order to the buffer
		if( ipp && ipp+3<end)
		{
			*ipp++ = (unsigned char)(ip[3]);
			*ipp++ = (unsigned char)(ip[2]);
			*ipp++ = (unsigned char)(ip[1]);
			*ipp++ = (unsigned char)(ip[0]);
		}
		return ip;
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress ()
	{	// implement little endian buffer format

		if( ipp && ipp+3<end)
		{
			ipaddress ip( ipp[0],ipp[1],ipp[2],ipp[3] );
			ipp+=4;
			return  ip;
		}
		return ((ulong)0);
	}
	///////////////////////////////////////////////////////////////////////////
	int64 operator = (const int64 lx)
	{	// implement little endian buffer format
		if( ipp && ipp+7<end)
		{
			*ipp++ = (unsigned char)(lx          );
			*ipp++ = (unsigned char)(lx  >> 0x08 );
			*ipp++ = (unsigned char)(lx  >> 0x10 );
			*ipp++ = (unsigned char)(lx  >> 0x18 );
			*ipp++ = (unsigned char)(lx  >> 0x20 );
			*ipp++ = (unsigned char)(lx  >> 0x28 );
			*ipp++ = (unsigned char)(lx  >> 0x30 );
			*ipp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	uint64 operator = (const uint64 lx)
	{	// implement little endian buffer format
		if( ipp && ipp+7<end)
		{
			*ipp++ = (unsigned char)(lx          );
			*ipp++ = (unsigned char)(lx  >> 0x08 );
			*ipp++ = (unsigned char)(lx  >> 0x10 );
			*ipp++ = (unsigned char)(lx  >> 0x18 );
			*ipp++ = (unsigned char)(lx  >> 0x20 );
			*ipp++ = (unsigned char)(lx  >> 0x28 );
			*ipp++ = (unsigned char)(lx  >> 0x30 );
			*ipp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	///////////////////////////////////////////////////////////////////////////
	operator int64 ()
	{	// implement little endian buffer format
		int64 lx=0;
		if( ipp && ipp+7<end)
		{	
			lx  = ((uint64)(*ipp++)        ); 
			lx |= ((uint64)(*ipp++) << 0x08);
			lx |= ((uint64)(*ipp++) << 0x10);
			lx |= ((uint64)(*ipp++) << 0x18);
			lx |= ((uint64)(*ipp++) << 0x20);
			lx |= ((uint64)(*ipp++) << 0x28);
			lx |= ((uint64)(*ipp++) << 0x30);
			lx |= ((uint64)(*ipp++) << 0x38);
		}
		return lx;
	}
	operator uint64 ()
	{	// implement little endian buffer format
		uint64 lx=0;
		if( ipp && ipp+7<end)
		{	
			lx  = ((uint64)(*ipp++)        ); 
			lx |= ((uint64)(*ipp++) << 0x08);
			lx |= ((uint64)(*ipp++) << 0x10);
			lx |= ((uint64)(*ipp++) << 0x18);
			lx |= ((uint64)(*ipp++) << 0x20);
			lx |= ((uint64)(*ipp++) << 0x28);
			lx |= ((uint64)(*ipp++) << 0x30);
			lx |= ((uint64)(*ipp++) << 0x38);
		}
		return lx;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{	
		if( c && ipp+strlen(c)+1 < end )
		{
			memcpy(ipp, c, strlen(c)+1);
			ipp+=strlen(c)+1;
		}
		return c;
	}
	operator const char*()
	{	// find the EOS
		// cannot do anything else then going through the array
		unsigned char *ix = ipp;
		while( ipp<end && ipp);
		if(ipp<end)
		{	// skip the eos in the buffer
			// it belongs to the string
			ipp++;
			return (char*)ix;
		}
		else
		{	// no eos, so there is no string
			ipp = ix;
			return NULL;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// string access with fixed string size
	///////////////////////////////////////////////////////////////////////////
	bool str2buffer(const char *c, size_t sz)
	{
		if( c && ipp+sz < end )
		{	
			size_t cpsz=sz;
			if( cpsz > strlen(c)+1 )
				cpsz = strlen(c)+1;
			memcpy(ipp, c, cpsz);
			ipp[cpsz-1] = 0;	// force an EOS
			ipp+=sz;
			return true;
		}
		return false;
	}
	bool buffer2str(char *c, size_t sz)
	{
		if( c && ipp+sz < end )
		{	
			memcpy(c, ipp, sz);
			c[sz-1] = 0;	// force an EOS
			ipp+=sz;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// read/write access for unsigned char arrays
	///////////////////////////////////////////////////////////////////////////
	bool write(const unsigned char *c, size_t sz)
	{
		if( c && ipp+sz < end )
		{	
			memcpy(ipp, c, sz);
			ipp+=sz;
			return true;
		}
		return false;
	}
	bool read(unsigned char *c, size_t sz)
	{
		if( c && ipp+sz < end )
		{	
			memcpy(c, ipp, sz);
			ipp+=sz;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer and "manual" contol
	// use with care
	///////////////////////////////////////////////////////////////////////////
	unsigned char* operator()()	{ return ipp; }

	bool step(int i)
	{	// can go backwards with negative offset
		if(ipp+i <= end && ipp+i >= start)
		{
			ipp+=i;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// the next will combine the buffer_iterator with the 
	// virtual streamable class allowing to derive
	// classes that are auto-assignable to buffers
	///////////////////////////////////////////////////////////////////////////
	const streamable& operator = (const streamable& s);


	///////////////////////////////////////////////////////////////////////////
	// I explicitely exclude int operators since the usage of ints 
	// should be generally banned from any portable storage scheme
	//
	// assigning an int will now cause an ambiquity 
	// which has to be solved by casting to a proper type with fixed size
	///////////////////////////////////////////////////////////////////////////
/*

	int operator = (const int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:	// 8bit systems should be quite rare
			(*this) = (char)i;
			break;
		case 2:	// 16bits might be not that common
			(*this) = (short)i;
			break;
		case 4:	// 32bits (still) exist
			(*this) = (long)i;
			break;
		case 8:	// 64bits are rising
			(*this) = (int64)i;
			break;
				// the rest is far future
		}
		return i;
	}
	operator int ()
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (char)(*this);
		case 2:
			return (short)(*this);
		case 4:
			return (long)(*this);
		case 8:
			return (int64)(*this);
		}
	}
	unsigned int operator = (const unsigned int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			(*this) = (char)i;
			break;
		case 2:
			(*this) = (short)i;
			break;
		case 4:
			(*this) = (long)i;
			break;
		case 8:
			(*this) = (int64)i;
			break;
		}
		return i;
	}
	operator unsigned int ()
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (char)(*this);
		case 2:
			return (short)(*this);
		case 4:
			return (long)(*this);
		case 8:
			return (int64)(*this);
		}
	}
*/
};


///////////////////////////////////////////////////////////////////////////////
// 
//
///////////////////////////////////////////////////////////////////////////////
class streamable
{
public:
	streamable()			{}
	virtual ~streamable()	{}

	buffer_iterator& operator=(buffer_iterator& bi)
	{
		this->fromBuffer(bi);
		return bi;
	}

	//////////////////////////////////////////////////////////////////////
	// return necessary size of buffer
	virtual size_t BufferSize() const =0;
	//////////////////////////////////////////////////////////////////////
	// writes content to buffer
	virtual bool toBuffer(buffer_iterator& bi) const   = 0;
	//////////////////////////////////////////////////////////////////////
	// reads content from buffer
	virtual bool fromBuffer(const buffer_iterator& bi) = 0;
};


inline const streamable& buffer_iterator::operator = (const streamable& s)
{
	s.toBuffer(*this);
	return s;
}















// Class for assigning/reading words from a buffer
class objW
{
	unsigned char *ip;
public:
	objW():ip(NULL)												{}
	objW(unsigned char* x):ip(x)								{}
	objW(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objW(char* x):ip((unsigned char*)x)							{}
	objW(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objW& init(unsigned char* x)			{ip=x;return *this;}
	objW& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objW& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objW& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned short() const
	{	return this->operator()();
	}
	unsigned short operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned short)(ip[0]))        )
					|( ((unsigned short)(ip[1])) << 0x08);
			
		}
		return 0;
	}

	objW& operator=(const objW& objw)
	{
		if(ip && objw.ip)
		{	
			memcpy(ip, objw.ip, 2);
		}
		return *this;
	}
	unsigned short operator=(unsigned short valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x00FF)          );
			ip[1] = (unsigned char)((valin & 0xFF00)  >> 0x08 );
		}
		return valin;
	}
};

// Class for assigning/reading dwords from a buffer
class objL
{
	unsigned char *ip;
public:
	objL():ip(NULL)												{}
	objL(unsigned char* x):ip(x)								{}
	objL(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objL(char* x):ip((unsigned char*)x)							{}
	objL(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objL& init(unsigned char* x)			{ip=x;return *this;}
	objL& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objL& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objL& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}


	operator unsigned long() const
	{	return this->operator()();
	}
	unsigned long operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[0]))        )
					|( ((unsigned long)(ip[1])) << 0x08)
					|( ((unsigned long)(ip[2])) << 0x10)
					|( ((unsigned long)(ip[3])) << 0x18);
			
		}
		return 0;
	}

	objL& operator=(const objL& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	unsigned long operator=(unsigned long valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x000000FF)          );
			ip[1] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[2] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[3] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

// Class for assigning/reading words from a buffer 
// without changing byte order
// necessary for IP numbers, which are always in network byte order
// it is implemented this way and I currently won't interfere with that
// even if it is quite questionable

// changes: 
// transfered all IP addresses in the programms to host byte order
// since IPs are transfered in network byte order
// we cannot use objL but its complementary here
class objLIP	
{
	unsigned char *ip;
public:
	objLIP():ip(NULL)											{}
	objLIP(unsigned char* x):ip(x)								{}
	objLIP(unsigned char* x,int pos):ip(x?x+pos:NULL)			{}
	objLIP(char* x):ip((unsigned char*)x)						{}
	objLIP(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objLIP& init(unsigned char* x)			{ip=x;return *this;}
	objLIP& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objLIP& init(char* x)					{ip=(unsigned char*)x;return *this;}
	objLIP& init(char* x,int pos)			{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned long() const
	{return this->operator()();
	}
	unsigned long operator()()	const
	{
		if(ip)
		{	
//			register unsigned long tmp;
//			memcpy(&tmp,ip,4);
//			return tmp;
			return	 ( ((unsigned long)(ip[3]))        )
					|( ((unsigned long)(ip[2])) << 0x08)
					|( ((unsigned long)(ip[1])) << 0x10)
					|( ((unsigned long)(ip[0])) << 0x18);


		}
		return 0;
	}
	objLIP& operator=(const objLIP& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	unsigned long operator=(unsigned long valin)
	{	
		if(ip)
		{
//			memcpy(ip, &valin, 4);

			ip[3] = (unsigned char)((valin & 0x000000FF)          );
			ip[2] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[1] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[0] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

class objB
{
	unsigned char *ip;
public:
	objB():ip(NULL)												{}
	objB(unsigned char* x):ip(x)								{}
	objB(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objB(char* x):ip((unsigned char*)x)							{}
	objB(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objB& init(unsigned char* x)			{ip=x;return *this;}
	objB& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objB& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objB& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned char() const
	{	return this->operator()();
	}
	unsigned char operator()()	const
	{
		if(ip)
		{	
			return ip[0];
		}
		return 0;
	}
	objB& operator=(const objB& objb)
	{
		if(ip && objb.ip)
		{	
			ip[0] = objb.ip[0];
		}
		return *this;
	}
	unsigned char operator=(unsigned char valin)
	{	
		if(ip)
		{
			ip[0] = valin;
		}
		return valin;
	}
};



// define declaration

#define RFIFOP(fd,pos) ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL))
#define RFIFOB(fd,pos) (objB( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOW(fd,pos) (objW( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOL(fd,pos) (objL( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOLIP(fd,pos) (objLIP( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOREST(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_size-session[fd]->rdata_pos):0))
#define RFIFOSPACE(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_max-session[fd]->rdata_size):0))
#define RBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define RBUFB(p,pos) (*((unsigned char*)RBUFP((p),(pos))))
#define RBUFW(p,pos) (objW((p),(pos)))
#define RBUFL(p,pos) (objL((p),(pos)))
#define RBUFLIP(p,pos) (objLIP((p),(pos)))

#define WFIFOSPACE(fd) ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata_max-session[fd]->wdata_size):0)

#define WFIFOP(fd,pos) ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL))
#define WFIFOB(fd,pos) (objB( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOW(fd,pos) (objW( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOL(fd,pos) (objL( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOLIP(fd,pos) (objLIP( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define WBUFB(p,pos) (*((unsigned char*)WBUFP((p),(pos))))
#define WBUFW(p,pos) (objW((p),(pos)))
#define WBUFL(p,pos) (objL((p),(pos)))
#define WBUFLIP(p,pos) (objLIP((p),(pos)))

#if defined(__INTERIX) || defined(CYGWIN) || defined(_WIN32)
	#undef FD_SETSIZE
#define FD_SETSIZE 4096
#endif	// __INTERIX


// Struct declaration
struct socket_data
{
	struct {
		bool connected : 1;			// true when connected
		bool remove : 1;			// true when to be removed
		bool marked : 1;			// true when deleayed removal is initiated (optional)
	}flag;

	unsigned char *rdata;		// buffer
	size_t rdata_max;			// size of buffer
	size_t rdata_size;			// size of data
	size_t rdata_pos;			// iterator within data

	time_t rdata_tick;			// tick of last read

	unsigned char *wdata;		// buffer
	size_t wdata_max;			// size of buffer
	size_t wdata_size;			// size of data

	unsigned long client_ip;	// just an ip in host byte order is enough (4byte instead of 16)

	int (*func_recv)(int);
	int (*func_send)(int);
	int (*func_parse)(int);
	int (*func_term)(int);
	int (*func_console)(char*);
	void* session_data;
};

// Data prototype declaration

extern struct socket_data *session[FD_SETSIZE];
extern size_t fd_max;


static inline bool session_isValid(int fd)
{
	return ( (fd>=0) && (fd<FD_SETSIZE) && (NULL!=session[fd]) );
}
static inline bool session_isActive(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.connected );
}	
	
static inline bool session_isRemoved(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.remove );
}
static inline bool session_isMarked(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.marked );
}
static inline bool session_Remove(int fd)
{	// force removal
	if( session_isValid(fd)	)
	{
		session[fd]->flag.connected = false;
		session[fd]->flag.marked	= false;
		session[fd]->flag.remove	= true;
	}
	return true;
}
static inline bool session_SetTermFunction(int fd, int (*term)(int))
{
	if( session_isValid(fd) )
		session[fd]->func_term = term;
	return true;
}

bool session_SetWaitClose(int fd, unsigned long timeoffset);
bool session_Delete(int fd);

// Function prototype declaration

int make_listen    (unsigned long ip, unsigned short port);
int make_connection(unsigned long ip, unsigned short port);

int realloc_fifo(int fd, size_t rfifo_size,size_t wfifo_size);

int WFIFOSET(int fd, size_t len);
int RFIFOSKIP(int fd, size_t len);

int do_sendrecv(int next);
void do_final_socket(void);

void socket_init(void);
void socket_final(void);

void flush_fifos();

int start_console(void);

void set_defaultparse(int (*defaultparse)(int));
void set_defaultconsoleparse(int (*defaultparse)(char*));

#endif	// _SOCKET_H_
