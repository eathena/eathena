#ifndef __INET__
#define __INET__


#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"


///////////////////////////////////////////////////////////////////////////////
// error functions
int sockerrno();
const char* sockerrmsg(int code);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//!! move to basesocket when added

// maximum accept time of a socket in microseconds
// it defines the maximum time after an incoming socket is accepted
// and ready to read from (it does not affect read/write, only accept)
// half a second is a good praktical value
#define SOCKETACCEPTTIME	500000


// define to use non-blocking sockets instead of blocking sockets
// nonblocking might be nicer but need periferal tests to
// assure complete read/write operations
//#define NONBLOCKINGSOCKETS


// variation of FD_SETSIZE is not that portable as described
// if possible do not reduce it
// if the number of aquired sockets exceed FD_SETSIZE,
// on unix you cannot use the sockets because they 
// don't fit into the fd_set structre
// on windows machines this does not matter since sockets 
// and FD_SETSIZE are not related as on unix
// on unix it is usually >1024 or windows standard is 64
// 64bit systems define FD_SETSIZE usually as 65536
// #define FD_SETSIZE set a number if necessary

// a worker thread is opened for each number of FD_WORKERSIZE sockets
// to get a more responsive socket behaviour with a huge number of sockets
// FD_WORKERSIZE can be defined smaller than FD_SETSIZE on unix
// so the possible FD_SETSIZE sockets are devided to 
// max (FD_SETSIZE/FD_WORKERSIZE) independend worker threads.
// on windows it is possible to just reduce the FD_SETSIZE 
// and have FD_WORKERSIZE = FD_SETSIZE for the same effect 
// at reduced memory usage because FD_SETSIZE only limits the
// size if the fd_set structure and not the number of usable sockets
#define FD_WORKERSIZE FD_SETSIZE



//////////////////////////////////////////////////////////////////////////
// BSD-compatible socket error codes for Win32
//////////////////////////////////////////////////////////////////////////
// since there is no way to access this structure in a portable way
// we need to headbang at a wall until one breaks			
//typedef	unsigned long	fd_mask;
#ifndef NBBY
#define	NBBY 8
#endif
#ifndef howmany
#define	howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif  
#ifndef NFDBITS
#define	NFDBITS	(sizeof (unsigned long) * NBBY)	// bits per mask
#endif


//////////////////////////////////////////////////////////////////////////
// socketlen type definition
//////////////////////////////////////////////////////////////////////////
#ifndef __socklen_t_defined
  #ifdef WIN32
    typedef int socklen_t;
  #elif CYGWIN
    // stupid cygwin is not standard conform
    #ifdef socklen_t
      #undef socklen_t
    #endif
    typedef int socklen_t;
  #else// normal unix with undefined socklen_t
    typedef unsigned int socklen_t;
  #endif
  #define __socklen_t_defined
#endif//__socklen_t_defined
typedef char* sockval_t;

//////////////////////////////////////////////////////////////////////////
#ifndef WIN32
// windows specific interface for non-windows systems
typedef int		SOCKET;
// function definitions
extern inline int closesocket(SOCKET fd)		
{
	return close(fd); 
}
extern inline int ioctlsocket(SOCKET fd, long cmd, unsigned long *arg)		
{
	return ioctl(fd,cmd,arg); 
}

#else
// unix specific interface for windows
extern inline int read(SOCKET fd, char*buf, int sz)		
{
	return recv(fd,buf,sz,0); 
}
extern inline int write(SOCKET fd, char*buf, int sz)	
{
	return send(fd,buf,sz,0); 
}

#endif
//!!
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
// BSD-compatible socket error codes for Win32
//////////////////////////////////////////////////////////////////////////

#if defined(WSAENOTSOCK) && !defined(ENOTSOCK)

#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
// #define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
// #define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE

// NOTE: these are not errno constants in UNIX!
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define TRY_AGAIN               WSATRY_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA

#endif

// shutdown() constants

#if defined(SD_RECEIVE) && !defined(SHUT_RD)
#  define SHUT_RD       SD_RECEIVE
#  define SHUT_WR       SD_SEND
#  define SHUT_RDWR     SD_BOTH
#endif


// max backlog value for listen()

#ifndef SOMAXCONN
#define SOMAXCONN -1
#endif






#if defined(__DARWIN__) || defined(WIN32)
typedef int psocklen;
#else
typedef socklen_t psocklen;
#endif




///////////////////////////////////////////////////////////////////////////////
// IP number stuff
///////////////////////////////////////////////////////////////////////////////
#ifndef INADDR_NONE
#define INADDR_NONE             INADDR_BROADCAST
#endif


///////////////////////////////////////////////////////////////////////////////
// class for ip numbers and helpers
// currently limited to IP4
///////////////////////////////////////////////////////////////////////////////
class ipaddress
{
private:
	///////////////////////////////////////////////////////////////////////////
	// class helper
	// does network initialisation and gets available system ips
	class _ipset_helper
	{	
		uint32	cAddr[16];	// ip addresses of local host (host byte order)
		uint	cCnt;		// # of valid ip addresses
	public:
		_ipset_helper();

		///////////////////////////////////////////////////////////////////////
		// number of found system ip's (w/o localhost)
		uint GetSystemIPCount()	const	{ return cCnt; }

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
	// 16 ipaddresses should be enough for practical purpose
	static _ipset_helper& gethelper()
	{
		static _ipset_helper iphelp;
		return iphelp;
	}
public:
	static ipaddress GetSystemIP(uint i=0)	{ return gethelper().GetSystemIP(i); }
	static uint GetSystemIPCount()			{ return gethelper().GetSystemIPCount(); }
	static bool isBindable(ipaddress ip);
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
	virtual ~ipaddress()			{}
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
    uchar& operator [] (int i);
    const uchar operator [] (int i) const;

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
	// boolean operators
	bool operator == (const ipaddress s) const { return cAddr==s.cAddr; }
	bool operator != (const ipaddress s) const { return cAddr!=s.cAddr; }
	bool operator == (const uint32 s) const { return cAddr==s; }
	bool operator != (const uint32 s) const { return cAddr!=s; }

	///////////////////////////////////////////////////////////////////////////
	// ip2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;
	operator const char*()	{ return this->tostring(NULL); }

	///////////////////////////////////////////////////////////////////////////
	// converts a string to an ip (host byte order)
	static ipaddress str2ip(const char *str);
	static bool str2ip(const char* str, ipaddress &addr, ipaddress &mask, ushort &port);
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
	virtual ~netaddress()	{}
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
	virtual ushort& port() { return cPort; }
	///////////////////////////////////////////////////////////////////////////
	// networkaddr2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;


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
	virtual ~subnetaddress()	{}
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
	// subnetworkaddr2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;

	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const subnetaddress s) const { return this->cMask==s.cMask && this->cAddr==s.cAddr && this->cPort==s.cPort; }
	bool operator != (const subnetaddress s) const { return this->cMask!=s.cMask || this->cAddr!=s.cAddr || this->cPort!=s.cPort; }

};


///////////////////////////////////////////////////////////////////////////////
// class for IP-sets 
// stores wan/lan ips with lan subnet and wan/lan ports
// can automatically fill default values
// does not automatically resolve wan addresses though
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

	virtual ~ipset()	{}
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
	bool init(const char *str);
	///////////////////////////////////////////////////////////////////////////
	// checks if the ip's are locally available and correct wrong entries
	// returns true on ok, false if something has changed
	bool checklocal();
	bool checkPorts();

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
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;

	///////////////////////////////////////////////////////////////////////////
	// boolean operators
	bool operator == (const ipset s) const { return this->cAddr==s.cAddr && this->cMask==s.cMask && this->cPort==s.cPort && wanaddr==s.wanaddr; }
	bool operator != (const ipset s) const { return this->cAddr!=s.cAddr || this->cMask!=s.cMask || this->cPort!=s.cPort || wanaddr!=s.wanaddr; }
};


///////////////////////////////////////////////////////////////////////////////
// string conversion
inline string<> tostring(const ipaddress& ip)		{ return ip.tostring(); }
inline string<> tostring(const netaddress& ip)		{ return ip.tostring(); }
inline string<> tostring(const subnetaddress& ip)	{ return ip.tostring(); }
inline string<> tostring(const ipset& ip)			{ return ip.tostring(); }


#endif//__INET__
