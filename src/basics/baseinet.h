#ifndef __INET__
#define __INET__


#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_inet();



///////////////////////////////////////////////////////////////////////////////
// library includes
#ifdef _MSC_VER
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

///////////////////////////////////////////////////////////////////////////////
/// error functions
#if defined(WIN32)
typedef ushort sa_family_t;
typedef ushort in_port_t;
#endif

///////////////////////////////////////////////////////////////////////////////
// error functions
int sockerrno();
const char* sockerrmsg(int code);

//////////////////////////////////////////////////////////////////////////
/// BSD-compatible socket error codes for Win32
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

/// shutdown() constants
#if defined(SD_RECEIVE) && !defined(SHUT_RD)
#  define SHUT_RD       SD_RECEIVE
#  define SHUT_WR       SD_SEND
#  define SHUT_RDWR     SD_BOTH
#endif

/// max backlog value for listen()
#ifndef SOMAXCONN
#define SOMAXCONN -1
#endif



///////////////////////////////////////////////////////////////////////////////
/// IP number stuff
///////////////////////////////////////////////////////////////////////////////
#ifndef INADDR_NONE
#define INADDR_NONE             INADDR_BROADCAST
#endif










///////////////////////////////////////////////////////////////////////////////
/// class for ip numbers and helpers.
/// currently limited to IP4
///////////////////////////////////////////////////////////////////////////////
class ipaddress
{
	friend string<> hostbyaddr(ipaddress ip);
private:
	///////////////////////////////////////////////////////////////////////////
	/// class helper.
	/// does network initialisation and gets available system ips
	class _ipset_helper
	{	
		uint32	cAddr[16];	///< ip addresses of local host (host byte order)
							///< 16 ipaddresses should be enough for practical purpose
		uint	cCnt;		///< # of valid ip addresses
	public:
		_ipset_helper() : cCnt(0)		{ this->init(); }
		~_ipset_helper();

		///////////////////////////////////////////////////////////////////////
		/// initialize
		void init();

		///////////////////////////////////////////////////////////////////////
		/// number of found system ip's (w/o localhost)
		uint GetSystemIPCount()	const	{ return this->cCnt; }

		///////////////////////////////////////////////////////////////////////
		/// get an address from the array, return loopback on error
		ipaddress GetSystemIP(uint i=0) const
		{
			if( i < this->cCnt )
				return this->cAddr[i];
			else if(this->cCnt>0)
				return this->cAddr[0];
			return (ipaddress)INADDR_LOOPBACK;
		}
	};
	///////////////////////////////////////////////////////////////////////////
	/// access on a singleton.
	static _ipset_helper& gethelper();
public:
	static void InitSystemIP()				{        ipaddress::gethelper().init(); }
	static ipaddress GetSystemIP(uint i=0)	{ return ipaddress::gethelper().GetSystemIP(i); }
	static uint GetSystemIPCount()			{ return ipaddress::gethelper().GetSystemIPCount(); }
	static bool isBindable(ipaddress ip);
	bool isBindable()	{ return ipaddress::isBindable(*this); }

protected:
	///////////////////////////////////////////////////////////////////////////
	/// class data
    union
    {
        uchar	bdata[4];
        uint32	cAddr;
    };
public:
	///////////////////////////////////////////////////////////////////////////
	/// standard constructor/destructor
    ipaddress():cAddr(INADDR_ANY)	{}
	virtual ~ipaddress()			{}
	///////////////////////////////////////////////////////////////////////////
	/// copy/assign (actually not really necessary)
    ipaddress(const ipaddress& a) : cAddr(a.cAddr)	{}
    const ipaddress& operator= (const ipaddress& a)	{ this->cAddr = a.cAddr; return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// construction set (needs explicite casts when initializing with 0)
    ipaddress(uint32 a):cAddr(a)	{}
	ipaddress(const char* str):cAddr(str2ip(str))	{}
	ipaddress(const string<>& str):cAddr(str2ip(str))	{}
    ipaddress(int a, int b, int c, int d)
	{
		this->cAddr = (a&0xFF) << 0x18
					| (b&0xFF) << 0x10
					| (c&0xFF) << 0x08
					| (d&0xFF);
	}
	///////////////////////////////////////////////////////////////////////////
	/// assignment set (needs explicite casts when assigning 0)
    const ipaddress& operator= (uint32 a)			{ this->cAddr = a; return *this; }
	const ipaddress& operator= (const char* str)	{ this->cAddr = str2ip(str); return *this; }
	const ipaddress& operator= (const string<>& str){ this->cAddr = str2ip(str); return *this; }
	

	bool init(const char *str)
	{
		this->cAddr = str2ip(str);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// bytewise access (writable and readonly)
    uchar& operator [] (int i);
    const uchar operator [] (int i) const;

	///////////////////////////////////////////////////////////////////////////
	/// pod access on the ip (host byte order)
    operator uint32() const	{ return this->cAddr; }

	///////////////////////////////////////////////////////////////////////////
	/// virtual access interface
	virtual uint32 addr() const { return this->cAddr; }
	virtual uint32& addr() { return this->cAddr; }
	///////////////////////////////////////////////////////////////////////////
	virtual const uint32 mask() const { return INADDR_BROADCAST; }
	virtual uint32& mask() { static uint32 dummy; return dummy=INADDR_BROADCAST; }
	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return 0; }
	virtual ushort& port() { static ushort dummy; return dummy=0; }

	///////////////////////////////////////////////////////////////////////////
	/// boolean operators
	bool operator == (const ipaddress s) const { return this->cAddr==s.cAddr; }
	bool operator != (const ipaddress s) const { return this->cAddr!=s.cAddr; }
	bool operator == (const uint32 s) const { return this->cAddr==s; }
	bool operator != (const uint32 s) const { return this->cAddr!=s; }

	///////////////////////////////////////////////////////////////////////////
	/// ip2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	/// not threadsafe
	virtual const char *tostring(char *buffer) const;
	operator const char*()	{ return this->tostring(NULL); }

	template<class T> friend stringoperator<T>& operator <<(stringoperator<T>& str, const ipaddress& ip);

	///////////////////////////////////////////////////////////////////////////
	/// converts a string to an ip (host byte order)
	static ipaddress str2ip(const char *str);
	static bool str2ip(const char* str, ipaddress &addr, ipaddress &mask, ushort &port);
};

// predeclaration necessary for gcc4
class ipset;	
///////////////////////////////////////////////////////////////////////////////
/// class for a network address. (compound of an ip address and a port number)
///////////////////////////////////////////////////////////////////////////////
class netaddress : public ipaddress
{
	friend class ipset;
protected:
	///////////////////////////////////////////////////////////////////////////
	/// class data
	ushort		cPort;
public:
	///////////////////////////////////////////////////////////////////////////
	/// standard constructor/destructor
    netaddress():ipaddress((uint32)INADDR_ANY),cPort(0)	{}
	virtual ~netaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	/// copy/assign (actually not really necessary)
    netaddress(const netaddress& a):ipaddress(a.cAddr),cPort(a.cPort){}
    const netaddress& operator= (const netaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// construction set
	netaddress(uint32 a, ushort p):ipaddress(a),cPort(p)	{}
	netaddress(uint32 ip):ipaddress(ip),cPort(0)	{}
	explicit netaddress(ushort p):ipaddress((uint32)INADDR_ANY),cPort(p)	{}
    netaddress(int a, int b, int c, int d, ushort p):ipaddress(a,b,c,d),cPort(p) {}
	netaddress(const char* str):ipaddress((uint32)INADDR_ANY),cPort(0)	{ init(str); }
	netaddress(const string<>& str):ipaddress((uint32)INADDR_ANY),cPort(0)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	/// assignment set
    const netaddress& operator= (uint32 a)			{ this->cAddr = a; return *this; }
	const netaddress& operator= (ushort p)			{ this->cPort = p; return *this; }
	const netaddress& operator= (const char* str)	{ init(str); return *this; }
	const netaddress& operator= (const string<>& str){ init(str); return *this; }
	bool init(const char *str)
	{	
		ipaddress mask; // dummy
		return ipaddress::str2ip(str, *this, mask, this->cPort);
	}

	///////////////////////////////////////////////////////////////////////////
	virtual const ushort port() const { return cPort; }
	virtual ushort& port() { return cPort; }
	///////////////////////////////////////////////////////////////////////////
	/// networkaddr2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	/// not threadsafe
	virtual const char *tostring(char *buffer) const;

	template<class T> friend stringoperator<T>& operator <<(stringoperator<T>& str, const netaddress& ip);
	template<class T> friend stringoperator<T>& operator <<(stringoperator<T>& str, const ipset& ip);

	///////////////////////////////////////////////////////////////////////////
	/// boolean operators
	bool operator == (const netaddress s) const { return this->cAddr==s.cAddr && this->cPort==s.cPort; }
	bool operator != (const netaddress s) const { return this->cAddr!=s.cAddr || this->cPort!=s.cPort; }
};



///////////////////////////////////////////////////////////////////////////////
/// class for a subnetwork address. (ip address, subnetmask and port number)
///////////////////////////////////////////////////////////////////////////////
class subnetaddress : public netaddress
{
	friend class ipset;
protected:
	///////////////////////////////////////////////////////////////////////////
	/// class data
	ipaddress cMask;
public:
	///////////////////////////////////////////////////////////////////////////
	/// standard constructor/destructor
    subnetaddress():cMask((uint32)INADDR_ANY)	{}
	virtual ~subnetaddress()	{}
	///////////////////////////////////////////////////////////////////////////
	/// copy/assign (actually not really necessary)
    subnetaddress(const subnetaddress& a):netaddress(a),cMask(a.cMask) {}
    const subnetaddress& operator= (const subnetaddress& a)	{ this->cAddr = a.cAddr; this->cPort=a.cPort; this->cMask=a.cMask; return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// construction set
	subnetaddress(uint32 a, uint32 m, ushort p):netaddress(a,p),cMask(m)	{}
	subnetaddress(netaddress a, ipaddress m):netaddress(a),cMask(m)	{}
	subnetaddress(const char* str):cMask((uint32)INADDR_ANY)	{ init(str); }
	subnetaddress(const string<>& str):cMask((uint32)INADDR_ANY)	{ init(str); }
	///////////////////////////////////////////////////////////////////////////
	/// assignment set
	const subnetaddress& operator= (const char* str)	{ init(str); return *this; }
	const subnetaddress& operator= (const string<>& str){ init(str); return *this; }
	bool init(const char *str)
	{	// format: <ip>/<mask>:<port>
		return ipaddress::str2ip(str, *this, this->cMask, this->cPort);
	}
	///////////////////////////////////////////////////////////////////////////
	/// virtual access interface
	virtual const uint32 mask() const { return cMask.addr(); }
	virtual uint32& mask() { return cMask.addr(); }
	///////////////////////////////////////////////////////////////////////////
	/// subnetworkaddr2string
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;

	template<class T> friend stringoperator<T>& operator <<(stringoperator<T>& str, const subnetaddress& ip);

	///////////////////////////////////////////////////////////////////////////
	/// boolean operators
	bool operator == (const subnetaddress s) const { return this->cMask==s.cMask && this->cAddr==s.cAddr && this->cPort==s.cPort; }
	bool operator != (const subnetaddress s) const { return this->cMask!=s.cMask || this->cAddr!=s.cAddr || this->cPort!=s.cPort; }

};


///////////////////////////////////////////////////////////////////////////////
/// class for IP-sets.
/// stores wan/lan ips with lan subnet and wan/lan ports
/// can automatically fill default values
/// does not automatically resolve wan addresses though
///////////////////////////////////////////////////////////////////////////////
class ipset : public subnetaddress
{
	///////////////////////////////////////////////////////////////////////////
	/// class data
	netaddress		wanaddr;
public:
	///////////////////////////////////////////////////////////////////////////
	/// construct/destruct
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
	explicit ipset(const ushort pt)
		: subnetaddress(ipaddress::GetSystemIP(0),(uint32)INADDR_ANY,pt),wanaddr((uint32)INADDR_ANY,pt)
	{}


	virtual ~ipset()	{}
	// can use default copy/assign here

	///////////////////////////////////////////////////////////////////////////
	/// construction set
	ipset(const char* str)		{ init(str); }
	ipset(const string<>& str)	{ init(str); }

	///////////////////////////////////////////////////////////////////////////
	/// assignment set
	const ipset& operator=(const char* str)		{ init(str); return *this; }
	const ipset& operator=(const string<>& str)	{ init(str); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// initializes from a format string
	/// automatically checks the ips for local usage
	bool init(const char *str);
	///////////////////////////////////////////////////////////////////////////
	/// checks if the ip's are locally available and correct wrong entries
	/// returns true on ok, false if something has changed
	bool checklocal();
	bool checkPorts();

public:
	///////////////////////////////////////////////////////////////////////////
	/// check if an given ip is LAN
	bool isLAN(const ipaddress ip) const
	{
		return ( (this->cAddr&this->cMask) == (ip.addr()&this->cMask) );
	}
	///////////////////////////////////////////////////////////////////////////
	/// check if an given ip is WAN
	bool isWAN(const ipaddress ip) const
	{
		return ( (this->cAddr&this->cMask) != (ip.addr()&this->cMask) );
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
	ipaddress WANIP()		{ return wanaddr.addr(); }
	ushort& WANPort()		{ return wanaddr.port(); }

	///////////////////////////////////////////////////////////////////////////
	/// returning as netaddresses only
	netaddress& LANAddr()	{ return *this;; }	
	netaddress& WANAddr()	{ return wanaddr; }

	///////////////////////////////////////////////////////////////////////////
	virtual string<> tostring() const;
	virtual ssize_t tostring(char *buffer, size_t sz) const;
	// not threadsafe
	virtual const char *tostring(char *buffer) const;

	template<class T> friend stringoperator<T>& operator <<(stringoperator<T>& str, const ipset& ip);

	///////////////////////////////////////////////////////////////////////////
	/// boolean operators
	bool operator == (const ipset s) const { return this->cAddr==s.cAddr && this->cMask==s.cMask && this->cPort==s.cPort && wanaddr==s.wanaddr; }
	bool operator != (const ipset s) const { return this->cAddr!=s.cAddr || this->cMask!=s.cMask || this->cPort!=s.cPort || wanaddr!=s.wanaddr; }
};


///////////////////////////////////////////////////////////////////////////////
/// string conversion
inline string<> tostring(const ipaddress& ip)		{ return ip.tostring(); }
inline string<> tostring(const netaddress& ip)		{ return ip.tostring(); }
inline string<> tostring(const subnetaddress& ip)	{ return ip.tostring(); }
inline string<> tostring(const ipset& ip)			{ return ip.tostring(); }


template<class T> stringoperator<T>& operator <<(stringoperator<T>& str, const ipaddress& ip);
template<class T> stringoperator<T>& operator <<(stringoperator<T>& str, const netaddress& ip);
template<class T> stringoperator<T>& operator <<(stringoperator<T>& str, const subnetaddress& ip);
template<class T> stringoperator<T>& operator <<(stringoperator<T>& str, const ipset& ip);

// give operation down to the wrapped basestring
template<class T> inline string<T>& operator <<(string<T>& str, const ipaddress& ip)		{ *str << ip; return str; }
template<class T> inline string<T>& operator <<(string<T>& str, const netaddress& ip)		{ *str << ip; return str; }
template<class T> inline string<T>& operator <<(string<T>& str, const subnetaddress& ip)	{ *str << ip; return str; }
template<class T> inline string<T>& operator <<(string<T>& str, const ipset& ip)			{ *str << ip; return str; }






///////////////////////////////////////////////////////////////////////////////
// some variables
extern ipaddress	iplocal;	///< stores local ip on startup, does not refresh automatically
const ipaddress		ipany((uint32)INADDR_ANY);
const ipaddress		iploopback((uint32)INADDR_LOOPBACK);
const ipaddress		ipnone((uint32)INADDR_NONE);


ipaddress hostbyname(const char* name);
string<> hostbyaddr(ipaddress ip);
string<> hostname(const char* name);





///////////////////////////////////////////////////////////////////////////////
/// ip filter class.
/// contains a vector of allow/deny rules based on ip/mask,
/// setup is done by sequential inserting the rule strings
class ipfilter : public global
{
	///////////////////////////////////////////////////////////////////////////
	/// ip rule entry.
	struct ipentry
	{
		bool		allow;
		ipaddress	address;
		ipaddress	mask;

		ipentry()
		{}
		ipentry(const ipaddress& ip, const ipaddress& mk, bool a=true)
			: allow(a), address(ip), mask(mk)
		{}

		bool operator==(const ipaddress& ip) const
		{
			return (this->address & this->mask) == (ip & this->mask);
		}
		bool operator==(const ipentry& a) const
		{
			return (this->address == a.address) && (this->mask==a.mask) && (this->allow==a.allow);
		}
		bool operator< (const ipentry& a) const
		{
			return (this < &a);
		}
	};

	vector<ipentry> iplist;

public:
	///////////////////////////////////////////////////////////////////////////
	/// constructor.
	ipfilter()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// destructor.
	~ipfilter()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// test ip on allowance.
	bool is_allowed(const ipaddress& ip) const
	{
		basics::vector<ipentry>::iterator iter(this->iplist);

		for(; iter; ++iter)
		{
			if( *iter == ip )
				return iter->allow;
		}
		// not allowed by default
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// insert ip rule string.
	/// format is:
	/// "clear"			- clears all entries
	/// "all"			- allows all addesses
	/// "allow <ip>"	- allows an ip
	/// "deny <ip>"		- denies an ip
	/// with <ip> beeing "ip" or "ip/mask" or "ip/mask bit count"
	void insert(const char* ipstr)
	{
		if( 0==strcasecmp(ipstr, "all") )
		{
			this->iplist.clear();
			this->iplist.append( ipentry(ipany, ipany) );
		}
		else if( 0==strcasecmp(ipstr, "clear") || 0==strcasecmp(ipstr, "none") )
		{
			this->iplist.clear();
		}
		else
		{	
			const bool a = (0==memcmp(ipstr,"allow", 5));
			const bool b = (a)?false:(0==memcmp(ipstr,"deny", 4));
			if( a ^ b )
			{
				const char* str = ipstr + ((a)?(5):(4));
				while(*str && *str==' ') ++str;
				// format is "ip" or "ip/mask" or "ip/mask bit count"
				//basics::CRegExp re("(\\d+.\\d+.\\d+.\\d+)\\s*\\/\\s*(?:(\\d+.\\d+.\\d+.\\d+)|(\\d+))?");
				subnetaddress addr(str);
				if( addr.mask() == ipany )	// don't allow 0.0.0.0 masks
					addr.mask() =  ipnone;	// set them to 255.255.255.255
				this->iplist.append( ipentry(addr.addr(), addr.mask(), a) );
			}
			else
			{	// some dummy default entry
				this->iplist.append( ipentry(ipany, ipnone, false) );
			}
		}
	}
	void insert(const basics::ipaddress& ip, const basics::ipaddress& mk, bool allow=false)
	{
		if( ip != ipany && mk != ipany )
			this->iplist.append( ipentry(ip, mk, allow) );
	}
	void insert(const basics::ipaddress& ip, bool allow=false)
	{
		if( ip != ipany )
			this->iplist.append( ipentry(ip, ipnone, allow) );
	}
	///////////////////////////////////////////////////////////////////////////
	/// assign ip rule string.
	const ipfilter& operator=(const char* str)	{ this->insert(str); return *this; }


	///////////////////////////////////////////////////////////////////////////
	/// explicit compare operator.
	/// for usage in parameters
	friend bool operator==(const ipfilter& a, const ipfilter& b)	{ return false; } 
	///////////////////////////////////////////////////////////////////////////
	/// explicit assignment operator.
	/// does basically append the assigned filter to this one
	/// for usage in parameters (copy constructor is default),
	const ipfilter& operator=(const ipfilter& ipf)
	{
		if( ipf.iplist.size() == 0 )
		{	// no entry in list, so there was a clear command
			this->iplist.clear();
		}
		else
		{	// otherwise append addresses but the dummy address
			basics::vector<ipentry>::iterator iter(ipf.iplist);
			for(; iter; ++iter)
			{
				if( iter->allow || iter->address!=ipany || iter->mask!=ipnone )
					this->iplist.append(*iter);
			}
		}
		return *this;
	}


};





NAMESPACE_END(basics)


#endif//__INET__
