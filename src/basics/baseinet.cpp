
#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseregex.h"
#include "baseexceptions.h"

#ifndef SINGLETHREAD
#  include "basethreads.h"   // for mutex
#endif
#include "baseinet.h"
#include "basesocket.h"

/// example for a static initializer.
/// the comma operator forces calling the init function and inititializer is set to the boolean
/// namespace
/// {
///   bool initialiser = (my_class:init(), true);
/// }

NAMESPACE_BEGIN(basics)



///////////////////////////////////////////////////////////////////////////////
// reentrant gethostby*() mess
#if defined(SINGLETHREAD)
#  define USE_GETHOSTBY
#else
#  if defined(WIN32)
#    define USE_GETHOSTBY
#  elif defined(__FreeBSD__) || defined(__DARWIN__)
#    define USE_GETIPNODEBY
#  elif defined(linux)
#    define USE_GETHOSTBY_R6
#  elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
#    define USE_LOCKED_GETHOSTBY
#  else  // hopefully the Sun-style call will work on all other systems as well
#    define USE_GETHOSTBY_R5
#  endif
#endif

#define GETHOSTBY_BUF_SIZE 4096


///////////////////////////////////////////////////////////////////////////////
// internet utilities
int sockerrno()
{
#ifdef WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

const char* sockerrmsg(int code)
{
    switch(code)
    {
    // only minimal set of most frequent/expressive errors; others go as "I/O error"
    case ENOTSOCK:          return "Invalid socket descriptor";
    case EMSGSIZE:          return "Message too long";
    case ENOPROTOOPT:
    case EPROTONOSUPPORT:
    case EPFNOSUPPORT:
    case EAFNOSUPPORT:      return "Protocol or address family not supported";
    case EADDRINUSE:        return "Address already in use";
    case EADDRNOTAVAIL:     return "Address not available";
    case ENETDOWN:          return "Network is down";
    case ENETUNREACH:       return "Network is unreachable";
    case ECONNRESET:        return "Connection reset by peer";
    case ETIMEDOUT:         return "Operation timed out";
    case ECONNREFUSED:      return "Connection refused";
    case EHOSTDOWN:         return "Host is down";
    case EHOSTUNREACH:      return "No route to host";

    // we always translate h_errno to ENOENT and simply show "host not found"
    case ENOENT:            return "Host not found";
    default: return unixerrmsg(code);
    }
}



///////////////////////////////////////////////////////////////////////////////
#if defined(USE_LOCKED_GETHOSTBY)
static mutex hplock;
#endif

ipaddress hostbyname(const char* name)
{
    ipaddress ip;
    hostent* hp;

    if ((ip = ntohl(::inet_addr(name))) != ipaddress(INADDR_NONE))
    {	// don't test ip[0]==0 to also allow network addresses to pass here
        return ip;
    }
    else
    {
#if defined(USE_GETHOSTBY)
        if ((hp = ::gethostbyname(name)) != NULL)
#elif defined(USE_LOCKED_GETHOSTBY)
        hplock.enter();
        if ((hp = ::gethostbyname(name)) != NULL)
#elif defined(USE_GETIPNODEBY)
        int herrno;
        if ((hp = ::getipnodebyname(name, AF_INET, 0, &herrno)) != NULL)
#elif defined(USE_GETHOSTBY_R6)
        int herrno;
        hostent result;
        char buf[GETHOSTBY_BUF_SIZE];
        if ((::gethostbyname_r(name, &result, buf, sizeof(buf), &hp, &herrno) == 0) && hp)
#elif defined(USE_GETHOSTBY_R5)
        int herrno;
        hostent result;
        char buf[GETHOSTBY_BUF_SIZE];
        if ((hp = ::gethostbyname_r(name, &result, buf, sizeof(buf), &herrno)) != NULL)
#endif
		{
			if (hp->h_addrtype == AF_INET)
			{
				uchar*a = (uchar*)hp->h_addr;
				ip =  (a[0]<<0x18)
					| (a[1]<<0x10)
					| (a[2]<<0x08)
					| (a[3]);
			}
#ifdef USE_GETIPNODEBY
            freehostent(hp);
#endif
        }
#if defined(USE_LOCKED_GETHOSTBY)
        hplock.leave();
#endif
    }

    return ip;
}

///////////////////////////////////////////////////////////////////////////////
string<> hostbyaddr(ipaddress ip)
{
    hostent* hp;
    string<> r;

#if defined(USE_GETHOSTBY)
    if ((hp = ::gethostbyaddr(pcchar(ip.bdata), sizeof(ip.bdata), AF_INET)) != NULL)
#elif defined(USE_LOCKED_GETHOSTBY)
    hplock.enter();
    if ((hp = ::gethostbyaddr(pcchar(ip.bdata), sizeof(ip.bdata), AF_INET)) != NULL)
#elif defined(USE_GETIPNODEBY)
    int herrno;
    if ((hp = ::getipnodebyaddr(pcchar(ip.bdata), sizeof(ip.bdata), AF_INET, &herrno)) != NULL)
#elif defined(USE_GETHOSTBY_R6)
    int herrno;
    hostent result;
    char buf[GETHOSTBY_BUF_SIZE];
    if ((::gethostbyaddr_r(pcchar(ip.bdata), sizeof(ip.bdata), AF_INET, &result, buf, sizeof(buf), &hp, &herrno) == 0) && hp)
#elif defined(USE_GETHOSTBY_R5)
    int herrno;
    hostent result;
    char buf[GETHOSTBY_BUF_SIZE];
    if ((hp = ::gethostbyaddr_r(pcchar(ip.bdata), sizeof(ip.bdata), AF_INET, &result, buf, sizeof(buf), &herrno)) != NULL)
#endif
    {
        r = hp->h_name;
#ifdef USE_GETIPNODEBY
        freehostent(hp);
#endif
    }
#if defined(USE_LOCKED_GETHOSTBY)
    hplock.leave();
#endif

    return r;
}

///////////////////////////////////////////////////////////////////////////////
string<> hostname(const char* name)
{
    hostent* hp;
    string<> r;

#if defined(USE_GETHOSTBY)
    if ((hp = ::gethostbyname(name)) != NULL)
#elif defined(USE_LOCKED_GETHOSTBY)
    hplock.enter();
    if ((hp = ::gethostbyname(name)) != NULL)
#elif defined(USE_GETIPNODEBY)
    int herrno;
    if ((hp = ::getipnodebyname(name, AF_INET, 0, &herrno)) != NULL)
#elif defined(USE_GETHOSTBY_R6)
    int herrno;
    hostent result;
    char buf[GETHOSTBY_BUF_SIZE];
    if ((::gethostbyname_r(name, &result, buf, sizeof(buf), &hp, &herrno) == 0) && hp)
#elif defined(USE_GETHOSTBY_R5)
    int herrno;
    hostent result;
    char buf[GETHOSTBY_BUF_SIZE];
    if ((hp = ::gethostbyname_r(name, &result, buf, sizeof(buf), &herrno)) != NULL)
#endif
    {
        r = hp->h_name;
#ifdef USE_GETIPNODEBY
        freehostent(hp);
#endif
    }
#if defined(USE_LOCKED_GETHOSTBY)
    hplock.leave();
#endif

    return r;
}




///////////////////////////////////////////////////////////////////////////////
// constructor for static _ipset_helper
// automatically initializes winsockets on the first access to an ipaddress
void ipaddress::_ipset_helper::init(void)
{
	cCnt = 0;
#ifdef WIN32
	char fullhost[255];
	// Start up windows networking
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	int err = WSAStartup(wVersionRequested, &wsaData);
	if ( err != 0 )
	{
		printf("SYSERR: WinSock not available!\n");
		throw exception("SYSERR: WinSock not available!\n");
	}
	if ( LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0 )
	{
		printf("SYSERR: WinSock version mismatch (2.0 or compatible required)!\n");
		throw exception("SYSERR: WinSock version mismatch (2.0 or compatible required)!");
	}

	if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR)
	{
		printf("No hostname defined!\n");
	} 
	else
	{
		uchar** a;
		unsigned int i;
		struct hostent* hent = gethostbyname(fullhost);
		if (hent == NULL) {
			printf("Cannot resolve our own hostname to a IP address");
			return;
		}
		a = (uchar**)hent->h_addr_list;
		for(i = 0; a[i] != NULL && i < 16; ++i) {
			cAddr[i] =	  (a[i][0]<<0x18)
						| (a[i][1]<<0x10)
						| (a[i][2]<<0x08)
						| (a[i][3]);		
		}
		cCnt = i;
	}
#else//not W32
	int pos;
	int fdes = socket(AF_INET, SOCK_STREAM, 0);
	char buf[2*16 * sizeof(struct ifreq)];
	struct ifconf ic;
	
	// The ioctl call will fail with Invalid Argument if there are more
	// interfaces than will fit in the buffer
	ic.ifc_len = sizeof(buf);
	ic.ifc_buf = buf;
	if(ioctl(fdes, SIOCGIFCONF, &ic) == -1)
	{
		printf("SIOCGIFCONF failed!\n");
	}
	else
	{
		for(pos = 0; pos<ic.ifc_len;   )
		{
			struct ifreq* ir = (struct ifreq *)(buf+pos);
			struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);
			if(a->sin_family == AF_INET) {
				u_long ad = ntohl(a->sin_addr.s_addr);
				if(ad != INADDR_LOOPBACK && ad != INADDR_ANY)
				{
					cAddr[cCnt++] = ad;
					if(cCnt >= 16)
						break;
				}
			}
#if (defined(BSD) && BSD >= 199103) || defined(_AIX) || defined(__APPLE__)
			pos += ir->ifr_addr.sa_len + sizeof(ir->ifr_name);
#else// not AIX or APPLE
			pos += sizeof(struct ifreq);
#endif//not AIX or APPLE
		}
	}
	closesocket(fdes);
#endif//not W32	
}
ipaddress::_ipset_helper::~_ipset_helper()
{
#ifdef WIN32
	WSACleanup();
#endif
}

///////////////////////////////////////////////////////////////////////////
/// need a singleton. this here is safe, 
/// we need it only once and destruction order is irrelevant,
/// CYGWIN does not support this function beeing inlined (crashes on access), 
/// maybe a general problem with static function variables
ipaddress::_ipset_helper& ipaddress::gethelper()
{
	static _ipset_helper iphelp;
	return iphelp;
}

///////////////////////////////////////////////////////////////////////////////
// ipaddress functions
bool ipaddress::isBindable(const ipaddress ip)
{	// check if an given IP is part of the system IP that can be bound to
	if( ip!=iploopback && gethelper().GetSystemIPCount() > 0 )
	{	// looping here is ok since the list is not large
		for(uint i=0; i<GetSystemIPCount(); ++i)
			if( ip==GetSystemIP(i) )
				return true;
		return false;
	}
	else
	{	// loopback or cannot determine system ip's, just accept all
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// bytewise access (writable and readonly)
uchar& ipaddress::operator [] (int i)                  
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
const uchar ipaddress::operator [] (int i) const
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


///////////////////////////////////////////////////////////////////////////////
// ip2string
string<> ipaddress::tostring() const
{	
	char buf[16];
	size_t sz = sprintf(buf, "%d.%d.%d.%d",
		(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF);
	return string<>(buf,sz);
}
ssize_t ipaddress::tostring(char *buffer, size_t sz) const
{	
	return snprintf(buffer, sz, "%d.%d.%d.%d",
		(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF);
}
const char *ipaddress::tostring(char *buffer) const
{	// usage of the static buffer is not threadsafe
	static char tmp[16];
	char *buf = (buffer) ? buffer : tmp;
	sprintf(buf, "%d.%d.%d.%d",
		(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF);
	return buf;
}

template<typename T>
stringoperator<T>& operator <<(stringoperator<T>& str, const ipaddress& ip)
{
	str << ((ip.cAddr>>0x18)&0xFF) << '.' <<
		   ((ip.cAddr>>0x10)&0xFF) << '.' <<
		   ((ip.cAddr>>0x08)&0xFF) << '.' <<
		   ((ip.cAddr      )&0xFF);
	return str;
}
// explicit instantiation
template stringoperator<char   >& operator<< (stringoperator<char   >& str, const ipaddress& ip);
template stringoperator<wchar_t>& operator<< (stringoperator<wchar_t>& str, const ipaddress& ip);


///////////////////////////////////////////////////////////////////////////////
// converts a string to an ip (host byte order)
ipaddress ipaddress::str2ip(const char *str)
{	// format: <ip>
	if(str)
	{
		while( stringcheck::isspace(*str) ) str++;
		// look up the name
		// this can take long time (i.e. until timeout looking up non-existing addresses)
		return hostbyname(str);
	}
	return GetSystemIP();
}
bool ipaddress::str2ip(const char* str, ipaddress &addr, ipaddress &mask, ushort &port)
{	// format: <ip>/<mask>:<port>
	bool ret = false;
	if(str)
	{
		char buffer[1024];
		const char *kp=NULL, *mp=NULL;
		kp = strchr(str,'/'); // the first ip/mask seperator
		mp=strchr(kp?kp:str,':'); // the second ip/port seperator
		if(kp && mp)
		{	// all data given
			// <ip>
			const uint len = ((size_t)(kp-str)>=sizeof(buffer))?(sizeof(buffer)-1):(kp-str);
			memcpy(buffer, str, len);
			buffer[len]=0;
			addr = ipaddress::str2ip(buffer);
			// <mask>
			if(kp<mp)
			{	// format is ok
				kp++;
				const uint len = ((size_t)(mp-kp)>=sizeof(buffer))?(sizeof(buffer)-1):(mp-kp);
				memcpy(buffer, kp, len);
				buffer[len]=0;
				if( strchr(buffer, '.') )
				{	// mask given as ip
					mask = ipaddress::str2ip(buffer);
				}
				else
				{	// mask given as number of mask bits
					const uint num = atoi(buffer);
					mask = (uint32)(0xFFFFFFFF << ((num<32)?(32-num):0));
				}
			}
			else
			{	// the mask seperator placement is wrong
				mask = (uint32)INADDR_NONE; // 255.255.255.255
			}
			// <port>
			port = atoi(mp+1);
		}
		else if(!kp && mp)
		{	// mo mask given
			// <ip>
			const uint len = ((size_t)(mp-str)>=sizeof(buffer))?(sizeof(buffer)-1):(mp-str);
			memcpy(buffer, str, len);
			buffer[len]=0;
			addr = ipaddress::str2ip(buffer);
			// default mask
			mask = (uint32)INADDR_NONE; // 255.255.255.255
			// <port>
			port = atoi(mp+1);
		}
		else if(kp && !mp)
		{	// no port given
			// <ip>
			const uint len = ((size_t)(kp-str)>=sizeof(buffer))?(sizeof(buffer)-1):(kp-str);
			memcpy(buffer, str, len);
			buffer[len]=0;
			addr = ipaddress::str2ip(buffer);
			// <mask>
			kp++;
			if( strchr(kp, '.') )
			{	// maske given as ip
				mask = ipaddress::str2ip(kp);
			}
			else
			{	// mask given as number of mask bits
				const uint num = atoi(kp);
				mask = (uint32)(0xFFFFFFFF << ((num<32)?(32-num):0));
			}
			// don't change the port
		}
		else
		{	// neither mask nor port given
			// <ip>
			addr = ipaddress::str2ip(str);
			// default mask
			mask = (uint32)INADDR_NONE; // 255.255.255.255
			// don't change the port
		}
		ret = true;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// networkaddr2string
string<> netaddress::tostring() const
{	
	char buf[32];
	size_t sz = sprintf(buf, "%d.%d.%d.%d:%d",
		(this->cAddr>>0x18)&0xFF,
		(this->cAddr>>0x10)&0xFF,
		(this->cAddr>>0x08)&0xFF,
		(this->cAddr      )&0xFF,
		 this->cPort);
	return string<>(buf,sz);
}
ssize_t netaddress::tostring(char *buffer, size_t sz) const
{	
	return snprintf(buffer, sz, "%d.%d.%d.%d:%d",
		(this->cAddr>>0x18)&0xFF,
		(this->cAddr>>0x10)&0xFF,
		(this->cAddr>>0x08)&0xFF,
		(this->cAddr      )&0xFF,
		 this->cPort);
}
const char *netaddress::tostring(char *buffer) const
{	// usage of the static buffer is not threadsafe
	static char tmp[32];
	char *buf = (buffer) ? buffer : tmp;
	sprintf(buf, "%d.%d.%d.%d:%d",
		(cAddr>>0x18)&0xFF,(cAddr>>0x10)&0xFF,(cAddr>>0x8)&0xFF,(cAddr)&0xFF,
		cPort);
	return buf;
}

template<typename T>
stringoperator<T>& operator <<(stringoperator<T>& str, const netaddress& ip)
{
	str << ((ip.addr()>>0x18)&0xFF) << '.' <<
		   ((ip.addr()>>0x10)&0xFF) << '.' <<
		   ((ip.addr()>>0x08)&0xFF) << '.' <<
		   ((ip.addr()      )&0xFF) << '.' <<
		   ( ip.port());
	return str;
}
// explicit instantiation
template stringoperator<char   >& operator<< (stringoperator<char   >& str, const netaddress& ip);
template stringoperator<wchar_t>& operator<< (stringoperator<wchar_t>& str, const netaddress& ip);

///////////////////////////////////////////////////////////////////////////////
// subnetworkaddr2string
string<> subnetaddress::tostring() const
{	
	char buf[64];
	size_t sz;
	if(this->cMask.addr()==INADDR_ANY)
		sz = sprintf(buf, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	else
		sz = sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort);
	return string<>(buf,sz);
}
ssize_t subnetaddress::tostring(char *buffer, size_t sz) const
{	
	if(this->cMask.addr()==INADDR_ANY)
		return snprintf(buffer, sz, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	else
		return snprintf(buffer, sz, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort);
}
const char *subnetaddress::tostring(char *buffer) const
{	// usage of the static buffer is not threadsafe
	static char tmp[64];
	char *buf = (buffer) ? buffer : tmp;
	if(this->cMask.addr()==INADDR_ANY)
		sprintf(buf, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	else
		sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort);
	return buf;
}
template<typename T>
stringoperator<T>& operator <<(stringoperator<T>& str, const subnetaddress& ip)
{
	if(ip.cMask.addr()==INADDR_ANY)
	{
		str << ((ip.addr()>>0x18)&0xFF) << '.' <<
			   ((ip.addr()>>0x10)&0xFF) << '.' <<
			   ((ip.addr()>>0x08)&0xFF) << '.' <<
			   ((ip.addr()      )&0xFF) << ':' <<
			   ( ip.port());
	}
	else
	{
		str << ((ip.addr()>>0x18)&0xFF) << '.' <<
			   ((ip.addr()>>0x10)&0xFF) << '.' <<
			   ((ip.addr()>>0x08)&0xFF) << '.' <<
			   ((ip.addr()      )&0xFF) << '/' <<
			   ((ip.mask()>>0x18)&0xFF) << '.' <<
			   ((ip.mask()>>0x10)&0xFF) << '.' <<
			   ((ip.mask()>>0x08)&0xFF) << '.' <<
			   ((ip.mask()      )&0xFF) << ':' <<
			   ( ip.port());
	}
	return str;
}
// explicit instantiation
template stringoperator<char   >& operator<< (stringoperator<char   >& str, const subnetaddress& ip);
template stringoperator<wchar_t>& operator<< (stringoperator<wchar_t>& str, const subnetaddress& ip);

///////////////////////////////////////////////////////////////////////////////
// ipset functions
// initializes from a format string
// automatically checks the ips for local usage
bool ipset::init(const char *str)
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

		if( !this->isBindable() )
		{	
			if( wanaddr.isBindable() )
			{	// we assumed it wrong
				swap(this->cAddr, wanaddr.cAddr);
				swap(this->cPort, wanaddr.cPort);
			}
			else
			{	// none is bindable, so we default the addresses
				if(wanaddr.addr() == INADDR_ANY )
				{	// possibly the wan address has been given
					wanaddr.cAddr = this->cAddr;
					wanaddr.cPort = this->cPort;
				}
				this->cAddr = ipaddress::GetSystemIP(0);
			}
		}
	}
	checklocal();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// checks if the ip's are locally available and correct wrong entries
// returns true on ok, false if something has changed
bool ipset::checklocal()
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
bool ipset::checkPorts()
{	// check for unset wan address/port
	if( wanaddr.port() == 0 ) 
		wanaddr.port() = this->port();
	if( this->port() == 0 ) 
		this->port() = wanaddr.port();
	return (this->port() != 0) ;
}

///////////////////////////////////////////////////////////////////////////////
string<> ipset::tostring() const
{	
	char buf[64];
	size_t sz;
	if(this->wanaddr.cAddr == INADDR_ANY)
	{	// have only one accessable ip
		sz = sprintf(buf, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	}
	else
	{	// have a full set
		sz = sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d, %d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF,
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort,
			(this->wanaddr.cAddr>>0x18)&0xFF,
			(this->wanaddr.cAddr>>0x10)&0xFF,
			(this->wanaddr.cAddr>>0x08)&0xFF,
			(this->wanaddr.cAddr      )&0xFF,
			 this->wanaddr.cPort);
	}
	return string<>(buf,sz);
}
ssize_t ipset::tostring(char *buffer, size_t sz) const
{	
	if(this->wanaddr.cAddr == INADDR_ANY)
	{	// have only one accessable ip
		return snprintf(buffer, sz, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	}
	else
	{	// have a full set
		return snprintf(buffer, sz, "%d.%d.%d.%d/%d.%d.%d.%d:%d, %d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF,
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort,
			(this->wanaddr.cAddr>>0x18)&0xFF,
			(this->wanaddr.cAddr>>0x10)&0xFF,
			(this->wanaddr.cAddr>>0x08)&0xFF,
			(this->wanaddr.cAddr      )&0xFF,
			 this->wanaddr.cPort);
	}
}
const char *ipset::tostring(char *buffer) const
{	// usage of the static buffer is not threadsafe
	static char tmp[64];
	char *buf = (buffer) ? buffer : tmp;
	if(this->wanaddr.cAddr == INADDR_ANY)
	{	// have only one accessable ip
		sprintf(buf, "%d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF, 
			 this->cPort);
	}
	else
	{	// have a full set
		sprintf(buf, "%d.%d.%d.%d/%d.%d.%d.%d:%d, %d.%d.%d.%d:%d",
			(this->cAddr>>0x18)&0xFF,
			(this->cAddr>>0x10)&0xFF,
			(this->cAddr>>0x08)&0xFF,
			(this->cAddr      )&0xFF,
			(this->cMask>>0x18)&0xFF,
			(this->cMask>>0x10)&0xFF,
			(this->cMask>>0x08)&0xFF,
			(this->cMask      )&0xFF,
			 this->cPort,
			(this->wanaddr.cAddr>>0x18)&0xFF,
			(this->wanaddr.cAddr>>0x10)&0xFF,
			(this->wanaddr.cAddr>>0x08)&0xFF,
			(this->wanaddr.cAddr      )&0xFF,
			 this->wanaddr.cPort);
	}
	return buf;
}

template<typename T>
stringoperator<T>& operator <<(stringoperator<T>& str, const ipset& ip)
{
	if(ip.wanaddr.addr()==INADDR_ANY)
	{
		str << ((ip.addr()>>0x18)&0xFF) << '.' <<
			   ((ip.addr()>>0x10)&0xFF) << '.' <<
			   ((ip.addr()>>0x08)&0xFF) << '.' <<
			   ((ip.addr()      )&0xFF) << ':' <<
			    (ip.port());
	}
	else
	{	// have a full set
		str << ((ip.addr()>>0x18)&0xFF) << '.'  <<
			   ((ip.addr()>>0x10)&0xFF) << '.'  <<
			   ((ip.addr()>>0x08)&0xFF) << '.'  <<
			   ((ip.addr()      )&0xFF) << '/'  <<
			   ((ip.mask()>>0x18)&0xFF) << '.'  <<
			   ((ip.mask()>>0x10)&0xFF) << '.'  <<
			   ((ip.mask()>>0x08)&0xFF) << '.'  <<
			   ((ip.mask()      )&0xFF) << ':'  <<
			    (ip.cPort)              << ','  << ' ' <<
			   ((ip.wanaddr.addr()>>0x18)&0xFF) << '.' <<
			   ((ip.wanaddr.addr()>>0x10)&0xFF) << '.' <<
			   ((ip.wanaddr.addr()>>0x08)&0xFF) << '.' <<
			   ((ip.wanaddr.addr()      )&0xFF) << ':' <<
			    (ip.wanaddr.cPort);
	}
	return str;
}
// explicit instantiation
template stringoperator<char   >& operator<< (stringoperator<char   >& str, const ipset& ip);
template stringoperator<wchar_t>& operator<< (stringoperator<wchar_t>& str, const ipset& ip);


//////////////////////////////////////////////////////////////////////////
// instantiate some fixed ip's
// the local assignment will also automatically turn on the network layer
// just by including this object file
ipaddress localip = ipaddress::GetSystemIP(0);





void test_inet()
{
#ifdef DEBUG


#endif//DEBUG
}


NAMESPACE_END(basics)
