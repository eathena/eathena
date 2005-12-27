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
// class for ip numbers and helpers
///////////////////////////////////////////////////////////////////////////////
class ipaddress
{
private:
	///////////////////////////////////////////////////////////////////////////
	// class helper
	// does network initialisation and gets available system ips
	template <uint C> class _ipset_helper
	{	
		uint32	cAddr[C];	// ip addresses of local host (host byte order)
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
			char buf[2*C * sizeof(struct ifreq)];
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
				struct ifreq* ir = (struct ifreq *)(buf+pos);
				struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);
				if(a->sin_family == AF_INET) {
					u_long ad = ntohl(a->sin_addr.s_addr);
					if(ad != INADDR_LOOPBACK) {
						cAddr[cCnt++] = ad;
						if(cCnt == C)
							break;
					}
				}
#if (defined(BSD) && BSD >= 199103) || defined(_AIX) || defined(__APPLE__)
				pos += ir->ifr_addr.sa_len + sizeof(ir->ifr_name);
#else// not AIX or APPLE
				pos += sizeof(struct ifreq);
#endif//not AIX or APPLE
			}
			closesocket(fdes);
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
	static ipaddress GetSystemIP(uint i=0)	{ return gethelper().GetSystemIP(i); }
	static uint GetSystemIPCount()			{ return gethelper().GetSystemIPCount(); }

	static bool isBindable(ipaddress ip)
	{	// check if an given IP is part of the system IP that can be bound to
		if( gethelper().GetSystemIPCount() > 0 )
		{
			for(uint i=0; i<GetSystemIPCount(); i++)
				if( ip==GetSystemIP(i) )
					return true;
			return false;
		}
		else
		{	// cannot determine system ip's, just accept all
			return true;
		}
	}
	bool isBindable()	{ return ipaddress::isBindable(*this); }

public:
	///////////////////////////////////////////////////////////////////////////
	// class data
    union
    {
        uchar	bdata[4];
        uint32	cAddr;
    };
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor/destructor
    ipaddress():cAddr(INADDR_ANY)	{}
	~ipaddress()					{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    ipaddress(const ipaddress& a) : cAddr(a.cAddr)	{}
    ipaddress& operator= (const ipaddress& a)	{ this->cAddr = a.cAddr; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set (needs explicite casts when initializing with 0)
    ipaddress(uint32 a):cAddr(a)	{}
	ipaddress(const char* str):cAddr(str2ip(str))	{}
    ipaddress(int a, int b, int c, int d)
	{
		cAddr =	 (a&0xFF) << 0x18
				|(b&0xFF) << 0x10
				|(c&0xFF) << 0x08
				|(d&0xFF);
	}
	///////////////////////////////////////////////////////////////////////////
	// assignment set (needs explicite casts when assigning 0)
    ipaddress& operator= (uint32 a)			{ cAddr = a; return *this; }
	ipaddress& operator= (const char* str)	{ cAddr = str2ip(str); return *this; }
	bool init(const char *str)
	{
		cAddr = str2ip(str);
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
    operator uint32() const	{ return cAddr; }

	///////////////////////////////////////////////////////////////////////////
	// virtual access interface
	virtual uint32 addr() const { return cAddr; }
	virtual uint32& addr() { return cAddr; }
	///////////////////////////////////////////////////////////////////////////
	virtual const uint32 mask() const { return INADDR_BROADCAST; }
	virtual uint32& mask() { static uint32 dummy; return dummy=INADDR_BROADCAST; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return 0; }
	virtual ushort& port() { static ushort dummy; return dummy=0; }

	///////////////////////////////////////////////////////////////////////////
	// ip2string
	virtual const char *getstring(char *buffer=NULL) const
	{	// usage of the static buffer is not threadsafe
		static char tmp[16];
		char *buf = (buffer) ? buffer : tmp;
		sprintf(buf, "%d.%d.%d.%d",
			(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF);
		return buf;
	}
	operator const char*()	{ return this->getstring(); }
	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const ipaddress s) const { return cAddr==s.cAddr; }
	bool operator != (const ipaddress s) const { return cAddr!=s.cAddr; }
	bool operator == (const uint32 s) const { return cAddr==s; }
	bool operator != (const uint32 s) const { return cAddr!=s; }

	///////////////////////////////////////////////////////////////////////////
	// converts a string to an ip (host byte order)
	static ipaddress str2ip(const char *str)
	{	// format: <ip>
		if(str)
		{
			struct hostent*h;
			while( isspace( ((unsigned char)(*str)) ) ) str++;
			// look up the name
			// this can take long time (i.e. until timeout looking up non-existing addresses)
			h = gethostbyname(str);
			if (h != NULL)
				return ipaddress( MakeDWord((unsigned char)h->h_addr[3], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[0]) );
			else
				return ipaddress( ntohl(inet_addr(str)) );
		}
		return GetSystemIP();
	}
	static bool str2ip(const char* str, ipaddress &addr, ipaddress &mask, ushort &port)
	{	// format: <ip>/<mask>:<port>
		bool ret = false;
		if(str)
		{
			char buffer[1024];
			const char *kp=NULL, *mp=NULL;
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
					mask = (uint32)INADDR_ANY;
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
				mask = (uint32)INADDR_ANY; // 0.0.0.0
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
				mask = (uint32)INADDR_ANY; // 0.0.0.0
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
class netaddress : public ipaddress
{
	friend class ipset;
protected:
	///////////////////////////////////////////////////////////////////////////
	// class data
	ushort		cPort;
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor/destructor
    netaddress():ipaddress((uint32)INADDR_ANY),cPort(0)	{}
	~netaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    netaddress(const netaddress& a):ipaddress(a.cAddr),cPort(a.cPort){}
    netaddress& operator= (const netaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set
	netaddress(uint32 a, ushort p):ipaddress(a),cPort(p)	{}
	netaddress(ushort p):ipaddress((uint32)INADDR_ANY),cPort(p)	{}
    netaddress(int a, int b, int c, int d, ushort p):ipaddress(a,b,c,d),cPort(p) {}
	netaddress(const char* str)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	// assignment set
    netaddress& operator= (uint32 a)		{ this->cAddr = a; return *this; }
	netaddress& operator= (ushort p)		{ this->cPort = p; return *this; }
	netaddress& operator= (const char* str)	{ init(str); return *this; }
	bool init(const char *str)
	{	
		ipaddress mask; // dummy
		return ipaddress::str2ip(str, *this, mask, this->cPort);
	}

	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return cPort; }
	virtual ushort& port()		 { return cPort; }
	///////////////////////////////////////////////////////////////////////////
	// networkaddr2string
	virtual const char *getstring(char *buffer=NULL) const
	{	// usage of the static buffer is not threadsafe
		static char tmp[32];
		char *buf = (buffer) ? buffer : tmp;
		sprintf(buf, "%d.%d.%d.%d:%d",
			(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF,
			cPort);
		return buf;
	}
	operator const char*() const { return this->getstring(); }
	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const netaddress s) const { return this->cAddr==s.cAddr && this->cPort==s.cPort; }
	bool operator != (const netaddress s) const { return this->cAddr!=s.cAddr || this->cPort!=s.cPort; }
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
    subnetaddress():cMask((uint32)INADDR_ANY)	{}
	~subnetaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy/assign (actually not really necessary)
    subnetaddress(const subnetaddress& a):netaddress(a),cMask(a.cMask) {}
    subnetaddress& operator= (const subnetaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; this->cMask=a.cMask; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// construction set
	subnetaddress(uint32 a, uint32 m, ushort p):netaddress(a,p),cMask(m)	{}
	subnetaddress(netaddress a, ipaddress m):netaddress(a),cMask(m)	{}
	subnetaddress(const char* str)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	// assignment set
	subnetaddress& operator= (const char* str)	{ init(str); return *this; }
	bool init(const char *str)
	{	// format: <ip>/<mask>:<port>
		return ipaddress::str2ip(str, *this, this->cMask, this->cPort);
	}
	///////////////////////////////////////////////////////////////////////////
	// virtual access interface
	virtual const uint32 mask() const { return cMask.cAddr; }
	virtual uint32& mask() { return cMask.cAddr; }
	///////////////////////////////////////////////////////////////////////////
	// networkaddr2string
	virtual const char *getstring(char *buffer=NULL) const
	{	// usage of the static buffer is not threadsafe
		static char tmp[64];
		char *buf = (buffer) ? buffer : tmp;
		if(this->cMask.cAddr==INADDR_ANY)
			sprintf(buf, "%d.%d.%d.%d:%d",
				(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF, 
				this->cPort);
		else
			sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
				(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF, 
				this->cMask[3],this->cMask[2],this->cMask[1],this->cMask[0], 
				this->cPort);
		return buf;
	}
	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const subnetaddress s) const { return this->cMask==s.cMask && this->cAddr==s.cAddr && this->cPort==s.cPort; }
	bool operator != (const subnetaddress s) const { return this->cMask!=s.cMask || this->cAddr!=s.cAddr || this->cPort!=s.cPort; }

};


///////////////////////////////////////////////////////////////////////////////
// class for IP-sets 
// stores wan/lan ips with lan subnet and wan/lan ports
// can automatically fill default values
///////////////////////////////////////////////////////////////////////////////
class ipset : public subnetaddress
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	netaddress		wanaddr;
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	// init with 0, need to
	ipset(const ipaddress lip = ipaddress::GetSystemIP(0),	// identify with the first System IP by default
		  const ipaddress lsu = (uint32)INADDR_ANY,			// 0.0.0.0
		  const ushort lpt    = 0,
		  const ipaddress wip = (uint32)INADDR_ANY,			// 0.0.0.0
		  const ushort wpt    = 0 )
		: subnetaddress(lip,lsu,lpt),wanaddr(wip,wpt)
	{}
	ipset(const uint32 lip,
		  const uint32 lsu,
		  const ushort lpt,
		  const uint32 wip,
		  const ushort wpt)
		: subnetaddress(lip,lsu,lpt),wanaddr(wip,wpt)
	{}

	ipset(const ushort lpt,
		  const ushort wpt)
		: subnetaddress(ipaddress::GetSystemIP(0),(uint32)INADDR_ANY,lpt),wanaddr((uint32)INADDR_ANY,wpt)
	{}
	ipset(const ushort pt)
		: subnetaddress(ipaddress::GetSystemIP(0),(uint32)INADDR_ANY,pt),wanaddr((uint32)INADDR_ANY,pt)
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
				{	// both have subnets or first has sub, second has none
					// assume order: "lan,wan"
					subnetaddress::init(buffer);
					wanaddr.init(ip);
				}
				else if(!jp && kp)
				{	// first has none, second has sub
					subnetaddress::init(ip);
					wanaddr.init(buffer);
				}
				else
				{	// no subnets; only take the first ip, default the second
					subnetaddress::init(buffer);
					wanaddr = netaddress((uint32)INADDR_ANY, this->cPort);
				}
			}
			else
			{	// only one given, assume it the lanip
				subnetaddress::init(str);
				wanaddr = netaddress((uint32)INADDR_ANY, this->cPort);
			}

			if( !this->isBindable() && wanaddr.isBindable() )
			{	// we assumed it wrong
				swap(this->cAddr, wanaddr.cAddr);
				swap(this->cPort, wanaddr.cPort);
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
		if(this->cAddr==INADDR_ANY || !this->isBindable() ) // not detected or not local
		{	// take the first system ip for lan
			this->cAddr = ipaddress::GetSystemIP(0);
			ret = false;
		}

		ret &= checkPorts();
		return ret;
	}
	bool checkPorts()
	{	// check for unset wan address/port
		if( wanaddr.port() == 0 ) 
			wanaddr.port() = this->port();
		if( this->port() == 0 ) 
			this->port() = wanaddr.port();
		return (this->port() != 0) ;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// check if an given ip is LAN
	bool isLAN(const ipaddress ip) const
	{
		return ( (this->cAddr&this->cMask) == (ip.cAddr&this->cMask) );
	}
	///////////////////////////////////////////////////////////////////////////
	// check if an given ip is WAN
	bool isWAN(const ipaddress ip) const
	{
		return ( (this->cAddr&this->cMask) != (ip.cAddr&this->cMask) );
	}
	///////////////////////////////////////////////////////////////////////////
	bool SetLANIP(const ipaddress ip)
	{	// a valid lan address should be bindable
		if( ipaddress::isBindable(ip) ) 
		{
			this->addr() = ip;
			return true;
		}
		return false;
	}
	bool SetWANIP(const ipaddress ip)
	{	
		wanaddr.addr() = ip;
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress LANIP() const	{ return this->cAddr; }
	ipaddress& LANMask()	{ return this->cMask; }
	ushort& LANPort()		{ return this->cPort; }
	ipaddress WANIP()		{ return wanaddr.cAddr; }
	ushort& WANPort()		{ return wanaddr.cPort; }

	///////////////////////////////////////////////////////////////////////////
	// returning as netaddresses only
	netaddress& LANAddr()	{ return *this;; }	
	netaddress& WANAddr()	{ return wanaddr; }

	///////////////////////////////////////////////////////////////////////////
	virtual const char *getstring(char *buffer=NULL) const
	{	// usage of the static buffer is not threadsafe
		static char tmp[64];
		char *buf = (buffer) ? buffer : tmp;
		if(this->cMask.cAddr == INADDR_ANY)
		{	// have only one accessable ip
			sprintf(buf, "%d.%d.%d.%d:%d",
				(this->cAddr>>0x18)&0xFF,(this->cAddr>>0x10)&0xFF,(this->cAddr>>0x8)&0xFF,(this->cAddr)&0xFF, 
				this->cPort);
		}
		else
		{	// have a full set
			sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d, %d.%d.%d.%d:%d",
				(this->cAddr>>0x18)&0xFF,(this->cAddr>>0x10)&0xFF,(this->cAddr>>0x8)&0xFF,(this->cAddr)&0xFF,
				(this->cMask>>0x18)&0xFF,(this->cMask>>0x10)&0xFF,(this->cMask>>0x8)&0xFF,(this->cMask)&0xFF,
				this->cPort,
				(wanaddr.cAddr>>0x18)&0xFF,(wanaddr.cAddr>>0x10)&0xFF,(wanaddr.cAddr>>0x8)&0xFF,(wanaddr.cAddr)&0xFF,
				wanaddr.cPort);
		}
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const ipset s) const { return this->cAddr==s.cAddr && this->cMask==s.cMask && this->cPort==s.cPort && wanaddr==s.wanaddr; }
	bool operator != (const ipset s) const { return this->cAddr!=s.cAddr || this->cMask!=s.cMask || this->cPort!=s.cPort || wanaddr!=s.wanaddr; }
};



///////////////////////////////////////////////////////////////////////////////
// predeclares
class streamable;






//////////////////////////////////////////////////////////////////////////
// allocators for write buffers
//
// dynamic one creates on its own
// static is using an external buffer for writing
//
// size is calculated with one element holding in advance
// so it can be used for cstring allocation
//////////////////////////////////////////////////////////////////////////
template <class T> class allocator_w : public global, public noncopyable
{
protected:
	T* cBuf;
	T* cEnd;
	T* cPtr;

	// std construct/destruct
	allocator_w()	: cBuf(NULL), cEnd(NULL), cPtr(NULL)				{}
	allocator_w(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cPtr(buf)	{}
	virtual ~allocator_w()	{}

	virtual bool checkwrite(size_t addsize)	=0;
};

///////////////////////////////////////////////////////////////////////////////
template <class T> class allocator_w_st : public allocator_w<T>
{
protected:
	allocator_w_st(T* buf, size_t sz) : allocator_w<T>(buf, sz)	{}
	allocator_w_st();						// no implementation here
	allocator_w_st(size_t sz);				// no implementation here
	virtual ~allocator_w_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		return ( this->cPtr && this->cPtr +addsize < this->cEnd );
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T> class allocator_w_dy : public allocator_w<T>
{
protected:
	allocator_w_dy(T* buf, size_t sz);		// no implementation here
	allocator_w_dy() : allocator_w<T>()			{}
	allocator_w_dy(size_t sz) : allocator_w<T>(){ checkwrite(sz); }
	virtual ~allocator_w_dy()				{ if(this->cBuf) delete[] this->cBuf; }
	virtual bool checkwrite(size_t addsize)
	{
		static const size_t quant1 = (size_t)(   64);
		static const size_t qmask1 = (size_t)(  ~63);
		static const size_t quant2 = (size_t)( 4096);
		static const size_t qmask2 = (size_t)(~4095);

		size_t sz = this->cPtr-this->cBuf + addsize;

		if( this->cBuf+sz >= this->cEnd  ||
			(this->cBuf+1024 >= this->cEnd && this->cBuf+4*sz < this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer for >1k strings
			if( sz <= 16 )
				sz = 16;
			else if( sz <= 32 )
				sz = 32;
			else if( sz <= 2048 )
				sz = (sz + quant1 - 1) & qmask1;
			else
				sz = (sz + quant2 - 1) & qmask2;

			char *tmp = new char[sz];
			if(this->cBuf)
			{
				memcpy(tmp, this->cBuf, this->cPtr-this->cBuf);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cPtr = tmp+(this->cPtr-this->cBuf);
			this->cBuf = tmp;
		}
		return (this->cEnd > this->cBuf);
	}
};

//////////////////////////////////////////////////////////////////////////
// allocators for read/write buffers
//
// dynamic one creates on its own
// static is using an external buffer for writing
//
//////////////////////////////////////////////////////////////////////////
template <class T> class allocator_rw : public global, public noncopyable
{
protected:
	T* cBuf;
	T* cEnd;
	T* cRpp;
	T* cWpp;

	// std construct/destruct
	allocator_rw()	: cBuf(NULL), cEnd(NULL), cRpp(NULL), cWpp(NULL)				{}
	allocator_rw(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cRpp(buf), cWpp(buf)	{}
	virtual ~allocator_rw()	{}

	virtual bool checkwrite(size_t addsize)	=0;
	virtual bool checkread(size_t addsize) const
	{
		return ( this->cRpp && this->cRpp + addsize <=this->cWpp );
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T> class allocator_rw_st : public allocator_rw<T>
{
protected:
	allocator_rw_st(T* buf, size_t sz) : allocator_rw<T>(buf, sz)	{}
	allocator_rw_st();						// no implementation here
	allocator_rw_st(size_t sz);				// no implementation here
	virtual ~allocator_rw_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		if( this->cWpp+addsize >= this->cEnd && this->cBuf < this->cRpp )
		{	// move the current buffer data when necessary and possible 
			memmove(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->cBuf+(this->cWpp-this->cRpp);
			this->cRpp = this->cBuf;
		}
		return ( this->cWpp && this->cWpp + addsize < this->cEnd );
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T> class allocator_rw_dy : public allocator_rw<T>
{
protected:
	allocator_rw_dy(T* buf, size_t sz);		// no implementation here
	allocator_rw_dy() : allocator_rw<T>()		{}
	allocator_rw_dy(size_t sz) : allocator_rw<T>(){ checkwrite(sz); }
	virtual ~allocator_rw_dy()				{ if(this->cBuf) delete[] this->cBuf; }
	virtual bool checkwrite(size_t addsize)
	{
		static const size_t quant1 = (size_t)(   64);
		static const size_t qmask1 = (size_t)(  ~63);
		static const size_t quant2 = (size_t)( 4096);
		static const size_t qmask2 = (size_t)(~4095);

		size_t sz = this->cPtr-this->cBuf + addsize;

		if( this->cBuf+sz > this->cEnd  ||
			(this->cBuf+8092 > this->cEnd && this->cBuf+4*sz < this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer for >8k buffers
			if( sz <= 16 )
				sz = 16;
			else if( sz <= 32 )
				sz = 32;
			else if( sz <= 2048 )
				sz = (sz + quant1 - 1) & qmask1;
			else
				sz = (sz + quant2 - 1) & qmask2;

			char *tmp = new char[sz];
			if(this->cBuf)
			{
				memcpy(tmp, this->cRpp, this->cWpp-this->cRpp);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+(this->cWpp-this->cRpp);
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->wpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			memmove(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->cBuf+(this->cWpp-this->cRpp);
			this->cRpp = this->cBuf;
		}
		return (this->cEnd > this->cBuf);
	}
};



//////////////////////////////////////////////////////////////////////////
// uchar buffer access
//
// uses seperated read and write pointer; pointers do not go rounds, 
// so it needs an explicit sync at r/w operations
//
//////////////////////////////////////////////////////////////////////////
class buffer_iterator : public global
{
	friend class buffer;

protected:
	mutable unsigned char *rpp;	// read pointer inside the buffer
	mutable unsigned char *wpp;	// write pointer inside the buffer
	unsigned char *start;		// pointer to the start of the buffer
	unsigned char *end;			// pointer to the end of the buffer


	buffer_iterator()
		: rpp(NULL), wpp(NULL), start(NULL), end(NULL)
	{}

	// don't define a copy constructor
	buffer_iterator(const buffer_iterator& bi);
public:
	buffer_iterator(unsigned char* b, size_t sz)
	{
		this->rpp  =b;
		this->wpp  =b;
		this->start=b;
		this->end  =b+sz;
	}
	virtual ~buffer_iterator()
	{
		this->rpp=this->wpp=this->start=this->end=NULL;
	}

	// assignment
	const buffer_iterator& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// access functions
	bool eof() const		{ return !(this->wpp && this->wpp<this->end); }
	size_t size() const		{ return this->end-this->start; }
	size_t datasize() const	{ return this->wpp-this->rpp; }
	size_t freesize() const	{ return this->end-this->wpp; }


	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		if( this->wpp+addsize > this->end && this->start < this->rpp )
		{	// move the current buffer data when necessary and possible 
			memmove(this->start, this->rpp, this->wpp-this->rpp);
			this->wpp = this->start+(this->wpp-this->rpp);
			this->rpp = this->start;
		}
		return ( this->wpp && this->wpp+addsize<=this->end );
	}
	virtual bool checkread(size_t addsize) const
	{

		return ( this->rpp && this->rpp+addsize<=this->wpp );
	}

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = ch;
		}
		return ch;
	}
	char operator = (const char ch)
	{	
		if( checkwrite(1) )
		{
			*this->wpp++ = (unsigned char)ch;
		}
		return ch;
	}
	bool assign_char(const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = ch;
			return true;
		}
		return false;
	}
	bool assign_char(const char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = (unsigned char)ch;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{	
		if( checkread(1) )
			return *this->rpp++;
		return 0;
	}
	operator char() const
	{	
		if( checkread(1) )
			return *this->rpp++;
		return 0;

	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->wpp++ = (unsigned char)(sr         ); 
			*this->wpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	short operator = (const short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->wpp++ = (unsigned char)(sr         ); 
			*this->wpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{	// implement little endian buffer format
		unsigned short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->rpp++)        ); 
			sr |= ((unsigned short)(*this->rpp++) << 0x08);
		}
		return sr;
	}
	operator short () const
	{	// implement little endian buffer format
		short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->rpp++)        ); 
			sr |= ((unsigned short)(*this->rpp++) << 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	sint32 operator = (const sint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{	// implement little endian buffer format
		unsigned long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->rpp++)        ); 
			ln |= ((uint32)(*this->rpp++) << 0x08);
			ln |= ((uint32)(*this->rpp++) << 0x10);
			ln |= ((uint32)(*this->rpp++) << 0x18);
		}
		return ln;
	}
	operator sint32 () const
	{	// implement little endian buffer format
		long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->rpp++)        ); 
			ln |= ((uint32)(*this->rpp++) << 0x08);
			ln |= ((uint32)(*this->rpp++) << 0x10);
			ln |= ((uint32)(*this->rpp++) << 0x18);
		}
		return ln;
	}
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
			*this->wpp++ = (unsigned char)(ln  >> 0x20 );
			*this->wpp++ = (unsigned char)(ln  >> 0x28 );
			*this->wpp++ = (unsigned char)(ln  >> 0x30 );
			*this->wpp++ = (unsigned char)(ln  >> 0x38 );
		}
		return ln;
	}
	long operator = (const long ln)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
			*this->wpp++ = (unsigned char)(ln  >> 0x20 );
			*this->wpp++ = (unsigned char)(ln  >> 0x28 );
			*this->wpp++ = (unsigned char)(ln  >> 0x30 );
			*this->wpp++ = (unsigned char)(ln  >> 0x38 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{	// implement little endian buffer format
		unsigned long lx=0;
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->rpp++)        ); 
			lx |= ((unsigned long)(*this->rpp++) << 0x08);
			lx |= ((unsigned long)(*this->rpp++) << 0x10);
			lx |= ((unsigned long)(*this->rpp++) << 0x18);
			lx |= ((unsigned long)(*this->rpp++) << 0x20);
			lx |= ((unsigned long)(*this->rpp++) << 0x28);
			lx |= ((unsigned long)(*this->rpp++) << 0x30);
			lx |= ((unsigned long)(*this->rpp++) << 0x38);
		}
		return lx;
	}
	operator long () const
	{	// implement little endian buffer format
		long lx=0;
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->rpp++)        ); 
			lx |= ((unsigned long)(*this->rpp++) << 0x08);
			lx |= ((unsigned long)(*this->rpp++) << 0x10);
			lx |= ((unsigned long)(*this->rpp++) << 0x18);
			lx |= ((unsigned long)(*this->rpp++) << 0x20);
			lx |= ((unsigned long)(*this->rpp++) << 0x28);
			lx |= ((unsigned long)(*this->rpp++) << 0x30);
			lx |= ((unsigned long)(*this->rpp++) << 0x38);
		}
		return lx;
	}
#endif
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{	// implement little endian buffer format
		// IPs are given in network byte order to the buffer
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ip[3]);
			*this->wpp++ = (unsigned char)(ip[2]);
			*this->wpp++ = (unsigned char)(ip[1]);
			*this->wpp++ = (unsigned char)(ip[0]);
		}
		return ip;
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{	// implement little endian buffer format
		if( checkread(4) )
		{
			ipaddress ip( this->rpp[0],this->rpp[1],this->rpp[2],this->rpp[3] );
			this->rpp+=4;
			return  ip;
		}
		return INADDR_ANY;
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(lx          );
			*this->wpp++ = (unsigned char)(lx  >> 0x08 );
			*this->wpp++ = (unsigned char)(lx  >> 0x10 );
			*this->wpp++ = (unsigned char)(lx  >> 0x18 );
			*this->wpp++ = (unsigned char)(lx  >> 0x20 );
			*this->wpp++ = (unsigned char)(lx  >> 0x28 );
			*this->wpp++ = (unsigned char)(lx  >> 0x30 );
			*this->wpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	uint64 operator = (const uint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(lx          );
			*this->wpp++ = (unsigned char)(lx  >> 0x08 );
			*this->wpp++ = (unsigned char)(lx  >> 0x10 );
			*this->wpp++ = (unsigned char)(lx  >> 0x18 );
			*this->wpp++ = (unsigned char)(lx  >> 0x20 );
			*this->wpp++ = (unsigned char)(lx  >> 0x28 );
			*this->wpp++ = (unsigned char)(lx  >> 0x30 );
			*this->wpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{	// implement little endian buffer format
		sint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->rpp++)        ); 
			lx |= ((uint64)(*this->rpp++) << 0x08);
			lx |= ((uint64)(*this->rpp++) << 0x10);
			lx |= ((uint64)(*this->rpp++) << 0x18);
			lx |= ((uint64)(*this->rpp++) << 0x20);
			lx |= ((uint64)(*this->rpp++) << 0x28);
			lx |= ((uint64)(*this->rpp++) << 0x30);
			lx |= ((uint64)(*this->rpp++) << 0x38);
		}
		return lx;
	}
	operator uint64 () const
	{	// implement little endian buffer format
		uint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->rpp++)        ); 
			lx |= ((uint64)(*this->rpp++) << 0x08);
			lx |= ((uint64)(*this->rpp++) << 0x10);
			lx |= ((uint64)(*this->rpp++) << 0x18);
			lx |= ((uint64)(*this->rpp++) << 0x20);
			lx |= ((uint64)(*this->rpp++) << 0x28);
			lx |= ((uint64)(*this->rpp++) << 0x30);
			lx |= ((uint64)(*this->rpp++) << 0x38);
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
		size_t sz= (c) ? strlen(c)+1 : 1;
		if( checkwrite(sz) )
		{
			if( c )
			{
				memcpy(this->wpp, c, sz);
				this->wpp+=sz;
			}
			else
				*this->wpp++=0;
		}
		return c;
	}
	operator const char*() const
	{	// find the EOS
		// cannot do anything else then going through the array
		unsigned char *ix = this->rpp;
		while( this->rpp<this->wpp && this->rpp );
		if(this->rpp<this->wpp)
		{	// skip the eos in the buffer
			// it belongs to the string
			this->rpp++;
			return (char*)ix;
		}
		else
		{	// no eos, so there is no string
			this->rpp = ix;
			return NULL;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// string access with fixed string size
	///////////////////////////////////////////////////////////////////////////
	bool str2buffer(const char *c, size_t sz)
	{
		if( checkwrite(sz) )
		{
			if( c )
			{
				size_t cpsz = ( cpsz > strlen(c)+1 ) ? sz : strlen(c)+1;
				memcpy(this->wpp, c, cpsz);
				this->wpp[cpsz-1] = 0;	// force an EOS
			}
			else
				memset(this->wpp, 0, sz);
			this->wpp+=sz;
			return true;
		}
		return false;
	}
	bool buffer2str(char *c, size_t sz) const
	{
		if( checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->rpp, sz);
				c[sz-1] = 0;	// force an EOS
				this->rpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// read/write access for unsigned char arrays
	///////////////////////////////////////////////////////////////////////////
	bool write(const unsigned char *c, size_t sz)
	{
		if( checkwrite(sz) )
		{
			if( c )
				memcpy(this->wpp, c, sz);
			this->wpp+=sz;
			return true;
		}
		return false;
	}
	bool read(unsigned char *c, size_t sz)
	{
		if( checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->rpp, sz);
				this->rpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer and "manual" contol
	// use with care
	///////////////////////////////////////////////////////////////////////////
	unsigned char* operator()(size_t offset=0, size_t size=0)
	{
		return get_rptr(offset); 
	}
	unsigned char* get_rptr(size_t offset=0, size_t size=0)
	{
		if( !checkread(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetReadpointer out of bound");
#else
			return NULL;
#endif
		return this->rpp+offset; 
	}
	unsigned char* get_wptr(size_t offset=0, size_t size=0)
	{
		if( !checkwrite(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetWritepointer out of bound");
#else
			return NULL;
#endif
		return this->wpp+offset; 
	}
	bool step_rptr(int i)
	{	// can go backwards with negative offset
		if(this->rpp+i <= this->wpp && this->rpp+i >= this->start)
		{
			this->rpp+=i;
			return true;
		}
		return false;
	}
	bool step_wptr(int i)
	{	// can go backwards with negative offset
		if(this->wpp+i <= this->end && this->wpp+i >= this->rpp)
		{
			this->wpp+=i;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// returning readable/writable and size checked portions of the buffer
	// that behave like a fixed buffer for inserting packets directly
	///////////////////////////////////////////////////////////////////////////
	buffer_iterator get_readbuffer(size_t sz)
	{
		if( checkread(sz) )
			return buffer_iterator(this->rpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->wpp, sz);
		else
			return buffer_iterator();
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
			(*this) = (sint32)i;
			break;
		case 8:	// 64bits are rising
			(*this) = (sint64)i;
			break;
				// the rest is far future
		}
		return i;
	}
	operator int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (char)(*this);
		case 2:
			return (short)(*this);
		case 4:
			return (sint32)(*this);
		case 8:
			return (sint64)(*this);
		}
	}
	unsigned int operator = (const unsigned int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			(*this) = (uchar)i;
			break;
		case 2:
			(*this) = (ushort)i;
			break;
		case 4:
			(*this) = (uint32)i;
			break;
		case 8:
			(*this) = (uint64)i;
			break;
		}
		return i;
	}
	operator unsigned int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (uchar)(*this);
		case 2:
			return (ushort)(*this);
		case 4:
			return (uint32)(*this);
		case 8:
			return (uint64)(*this);
		}
	}
*/
};



template <size_t SZ> class buffer_fixed : public buffer_iterator
{
	unsigned char cBuf[ (SZ<=16) ? 16 : (SZ<=32) ? 32 : (SZ<=2048) ? (SZ+63)&(~63) : (SZ+4095)&(~4095) ];
public:
	buffer_fixed() : buffer_iterator(cBuf, (SZ<=16) ? 16 : (SZ<=32) ? 32 : (SZ<=2048) ? (SZ+63)&(~63) : (SZ+4095)&(~4095))
	{}
	virtual ~buffer_fixed()
	{}

	// copy/assignment
	buffer_fixed(const buffer_fixed& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer_fixed& operator=(const buffer_fixed& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer_fixed(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer_fixed& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}



	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	char operator = (const char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{
		return this->buffer_iterator::operator unsigned char();
	}
	operator char() const
	{
		return this->buffer_iterator::operator char();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	short operator = (const short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{
		return this->buffer_iterator::operator unsigned short();
	}
	operator short () const
	{
		return this->buffer_iterator::operator short();
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	sint32 operator = (const sint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{
		return this->buffer_iterator::operator uint32();
	}
	operator sint32 () const
	{
		return this->buffer_iterator::operator sint32();
	}
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	long operator = (const long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{
		return this->buffer_iterator::operator unsigned long();
	}
	operator long () const
	{
		return this->buffer_iterator::operator long();
	}
#endif
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{
		return this->buffer_iterator::operator=(ip);
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{
		return this->buffer_iterator::operator ipaddress();
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	uint64 operator = (const uint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{
		return this->buffer_iterator::operator uint64();
	}
	operator uint64 () const
	{
		return this->buffer_iterator::operator sint64();
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{
		return this->buffer_iterator::operator=(c);
	}
	operator const char*() const
	{
		return this->buffer_iterator::operator const char*();
	}
};

class buffer : public buffer_iterator
{
public:
	buffer() : buffer_iterator()
	{}
	virtual ~buffer()
	{
		if(this->start) delete[] this->start;
		this->rpp=this->wpp=this->start=this->end=NULL;
	}

	// copy/assignment
	buffer(const buffer& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer& operator=(const buffer& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		static const size_t quant1 = (size_t)(   64);
		static const size_t qmask1 = (size_t)(  ~63);
		static const size_t quant2 = (size_t)( 4096);
		static const size_t qmask2 = (size_t)(~4095);

		size_t sz = this->wpp-this->rpp + addsize;

		if( this->start+sz > this->end ||
			(this->start+8092 > this->end && this->start+4*sz < this->end) )
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer
			if( sz <= 16 )
				sz = 16;
			else if( sz <= 32 )
				sz = 32;
			else if( sz <= 2048 )
				sz = (sz + quant1 - 1) & qmask1;
			else
				sz = (sz + quant2 - 1) & qmask2;

			unsigned char *tmp = new unsigned char[sz];
			if(this->start)
			{
				memcpy(tmp, this->rpp, this->wpp-this->rpp);
				delete[] this->start;
			}
			this->end = tmp+sz;
			this->wpp = tmp+(this->wpp-this->rpp);
			this->rpp = tmp;
			this->start = tmp;
		}
		else if( this->wpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			memmove(this->start, this->rpp, this->wpp-this->rpp);
			this->wpp = this->start+(this->wpp-this->rpp);
			this->rpp = this->start;
		}
		// else everything ok
		return (this->end > this->start);
	}

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	char operator = (const char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{
		return this->buffer_iterator::operator unsigned char();
	}
	operator char() const
	{
		return this->buffer_iterator::operator char();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	short operator = (const short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{
		return this->buffer_iterator::operator unsigned short();
	}
	operator short () const
	{
		return this->buffer_iterator::operator short();
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	sint32 operator = (const sint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{
		return this->buffer_iterator::operator uint32();
	}
	operator sint32 () const
	{
		return this->buffer_iterator::operator sint32();
	}
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	long operator = (const long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{
		return this->buffer_iterator::operator unsigned long();
	}
	operator long () const
	{
		return this->buffer_iterator::operator long();
	}
#endif
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{
		return this->buffer_iterator::operator=(ip);
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{
		return this->buffer_iterator::operator ipaddress();
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	uint64 operator = (const uint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{
		return this->buffer_iterator::operator uint64();
	}
	operator uint64 () const
	{
		return this->buffer_iterator::operator sint64();
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{
		return this->buffer_iterator::operator=(c);
	}
	operator const char*() const
	{
		return this->buffer_iterator::operator const char*();
	}

	///////////////////////////////////////////////////////////////////////////
	// returning readable/writable and size checked portions of the buffer
	// that behave like a fixed buffer for inserting packets directly
	///////////////////////////////////////////////////////////////////////////
	buffer_iterator get_readbuffer(size_t sz)
	{
		if( checkread(sz) )
			return buffer_iterator(this->rpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->wpp, sz);
		else
			return buffer_iterator();
	}
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
		if( bi.checkread(this->BufferSize()) )
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
	if( checkwrite(s.BufferSize()) )
		s.toBuffer(*this);
	return s;
}


/*
extern inline void buffer_test()
{
	try
	{
		size_t i;

		buffer ba;
		unsigned char buf[16];
		buffer_iterator bb(buf, sizeof(buf));
		
		ba = (unsigned long)1;

		for(i=0; i<5; i++)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=5; i<10; i++)
			printf("%02x ", (unsigned short)bb);
		printf("\n");

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");


		for(i=5; i<10; i++)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=0; i<10; i++)
			printf("%i", (unsigned short)bb);
		printf("\n");
	
	}
	catch( CException e )
	{
		printf((const char*)e);
	}
}
*/


///////////////////////////////////////////////////////////////////////////////
template < class alloc=allocator_w_dy<char> > class stringbuffer : public alloc
{
public:
	stringbuffer() : alloc()									{}
	stringbuffer(size_t sz) : alloc(sz)							{}
	stringbuffer(char*buf, size_t sz) : alloc(buf, sz)			{}
	virtual ~stringbuffer()										{}

	template <class T> stringbuffer(const stringbuffer<T>& sb) : alloc(sb.length())
	{
		*this<<sb;
	}

	// standard string functions
	operator const char*() const	{ return this->cBuf; }
	const char* c_str()	const		{ return this->cBuf; }
	size_t length()	const			{ return (this->cPtr-this->cBuf); }
	void clear()					{ this->cPtr=this->cBuf; if(this->cPtr) *this->cPtr=0; }

	// operators
	template <class T> stringbuffer& operator =(const T t)
	{
		this->cPtr = this->cBuf;
		return *this<<t;
	}
	template <class T> stringbuffer& operator +=(const T t)
	{
		return *this<<t;
	}
	template <class T> stringbuffer< allocator_w_dy<char> > operator +(const T t)
	{
		return (stringbuffer< allocator_w_dy<char> >(*this)<< t);
	}


	// operator realisations for supported types
	template <class T> stringbuffer& operator <<(const stringbuffer<T>& sb)
	{
		if(sb.length())
		{
			if( this->checkwrite(sb.length()) )
			{
				memcpy(this->cPtr, sb.c_str(), sb.length());
				this->cPtr += sb.length();
			}
			if(this->cPtr) *this->cPtr=0;
		}
		return *this;
	}
	stringbuffer& operator <<(const char* ip)
	{
		if(ip)
		{
			while( *ip && this->checkwrite(1) )
				*this->cPtr++ = *ip++;
			if(this->cPtr) *this->cPtr=0;
		}
		return *this;
	}
	stringbuffer& operator <<(char ch)
	{
		if( ch && this->checkwrite(1) )
		{
			*this->cPtr++ = ch;
			*this->cPtr=0;
		}
		return *this;
	}
	stringbuffer& operator <<(int v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%i", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	stringbuffer& operator <<(unsigned int v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%u", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	stringbuffer& operator <<(long v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%li", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	stringbuffer& operator <<(unsigned long v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%lu", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	stringbuffer& operator <<(ipaddress ip)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, ip.getstring());
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}

	stringbuffer& operator <<(double v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%lf", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
};



//?? static string	-> cntptr, stc char	-> TPtrCount< stringbuffer< allocator_w_st<char> > >
// global string	-> cntptr, dyn char	-> TPtrAutoCount< stringbuffer< allocator_w_dy<char> > >
// dynamic string	-> cpwptr, dyn char	-> TPtrAutoRef< stringbuffer< allocator_w_dy<char> > >


template <class X> class string
{
//	friend class string< TPtrCount< stringbuffer< allocator_w_st<char> > > >;
	friend class string< TPtrAutoCount< stringbuffer< allocator_w_dy<char> > > >;
	friend class string< TPtrAutoRef< stringbuffer< allocator_w_dy<char> > > >;
protected:
	X strptr;
public:
	string()
	{}

	// standard string functions
	operator const char*() const	{ return strptr->c_str(); }
	const char* c_str()	const		{ return strptr->c_str(); }
	size_t length()	const			{ return strptr->length(); }
	void clear()					{ return strptr->clear(); }


	// assignment operators
	template<class T> string& operator =(const string<T> sb)
	{
		strptr = sb.strptr;
		return *this;
	}

	string& operator =(const char* t)
	{
		strptr->operator=(t);
		return *this;
	}
	string& operator =(char t)
	{
		strptr->operator=(t);
		return *this;
	}
	string& operator =(int t)
	{
		strptr->operator=(t);
		return *this;
	}
	string& operator =(unsigned int t)
	{
		strptr->operator=(t);
		return *this;
	}
	string& operator =(double t)
	{
		strptr->operator=(t);
		return *this;
	}

	template <class T> string& operator +=(const string<T> t)
	{
		strptr->operator+=(*t.strptr);
		return *this;
	}
	template <class T> string& operator +=(const T t)
	{
		strptr->operator+=(t);
		return *this;
	}
	template <class T> string operator +(const string<T> t)
	{
		string a(this);
		a.strptr->operator+=(*t.strptr);
		return a;
	}
	template <class T> string operator +(const T t)
	{
		string a(this);
		a.strptr->operator+=(t);
		return a;
	}
	template <class T> string& operator <<(const string<T> t)
	{
		strptr->operator<<(*t.strptr);
	}
	template <class T> string& operator <<(const T t)
	{
		strptr->operator<<(t);
		return *this;
	}
};


//typedef string< TPtrCount< stringbuffer< allocator_w_st<char> > > >     sstring;
typedef string< TPtrAutoCount< stringbuffer< allocator_w_dy<char> > > > gstring;
typedef string< TPtrAutoRef< stringbuffer< allocator_w_dy<char> > > >   dstring;




extern inline void stringbuffer_test()
{

	{
		stringbuffer<allocator_w_dy<char> > sa, sb, sc;
		gstring ga, gb, gc;
		dstring da, db, dc;


		da = "hallo";
		db = "test";
		dc = da;

		ga = da;

		ga += "1111";
		da += "2222";

		sa = ga;

		gb = sa;
	}







	size_t i, sz;
	char buffer1[1024];

	char buffer2[1024];
	stringbuffer< allocator_w_st<char> > sa(buffer2, sizeof(buffer2));
	stringbuffer< allocator_w_dy<char> > sb;
	MiniString a;




	sa = "hallo";
	sb = "hallo";
	sa << "test" << 19999;

	sb = sa + 111111;

	ulong tick = gettick();
	for(i=0; i<100000; i++)
		sz = snprintf(buffer1, sizeof(buffer1), "hallo %i ballo %i no %lf", i, i*2/3+i, ((double)i)*1.3);
	printf("%i\n", gettick()-tick);

	tick = gettick();
	for(i=0; i<100000; i++)
		a = MiniString("hallo ") + (int)i + " ballo " + (int)(i*2/3+i) + " no " +(((double)i)*1.3);
	printf("%i\n", gettick()-tick);


	tick = gettick();
	for(i=0; i<100000; i++)
	{
		sa.clear();
		sa << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("%i\n", gettick()-tick);
	
	tick = gettick();
	for(i=0; i<100000; i++)
	{
		sb.clear();
		sb << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("%i\n", gettick()-tick);
	printf("");

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

	
	operator ipaddress() const
	{
		return this->operator()();
	}
	ipaddress operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[3]))        )
					|( ((unsigned long)(ip[2])) << 0x08)
					|( ((unsigned long)(ip[1])) << 0x10)
					|( ((unsigned long)(ip[0])) << 0x18);


		}
		return INADDR_ANY;
	}
	objLIP& operator=(const objLIP& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	ipaddress operator=(ipaddress valin)
	{	
		if(ip)
		{
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


// Struct declaration
struct socket_data
{
	struct {
		bool connected : 1;		// true when connected
		bool remove : 1;		// true when to be removed
		bool marked : 1;		// true when deleayed removal is initiated (optional)
	}flag;

	time_t rdata_tick;			// tick of last read

	buffer rbuffer;
	buffer wbuffer;

	unsigned char *rdata;		// buffer
	size_t rdata_max;			// size of buffer
	size_t rdata_size;			// size of data
	size_t rdata_pos;			// iterator within data

	unsigned char *wdata;		// buffer
	size_t wdata_max;			// size of buffer
	size_t wdata_size;			// size of data

	ipaddress client_ip;	// just an ip in host byte order is enough (4byte instead of 16)

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
