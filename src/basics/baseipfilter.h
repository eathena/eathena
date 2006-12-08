#ifndef __BASEIPFILTER__
#define __BASEIPFILTER__


#include "baseinet.h"
#include "basesync.h"

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_ipfilter();

///////////////////////////////////////////////////////////////////////////
/// entry in allow/deny lists.
/// store netip and mask of a allow/deny rule
struct iprule_t
{
	ipaddress addr;
	ipaddress mask;

	iprule_t()	{}
	iprule_t(const ipaddress& i, const ipaddress& m=ipnone)
		: addr(i&m), mask(m)
	{}

	bool operator==(const iprule_t& a) const
	{
		return (this->addr&a.mask)==(a.addr&this->mask);
	}
	bool operator< (const iprule_t& a) const
	{
		return (this->addr&a.mask)< (a.addr&this->mask);
	}
	bool operator==(const ipaddress& i) const
	{
		return (this->addr) == (i&this->mask);
	}
	bool operator< (const ipaddress& i) const
	{
		return (this->addr) < (i&this->mask);
	}
};

///////////////////////////////////////////////////////////////////////////
/// abstract rule list.
/// sorted vector of rule entries 
/// with conversion interface to add new rules,
/// should not be used alone
struct iprulelist_t : protected Mutex
{
	slist<iprule_t> list;
public:
	iprulelist_t()
	{}
	iprulelist_t(const iprulelist_t& a)
		: list(a.list)
	{}
	const iprulelist_t& operator=(const iprulelist_t& a)
	{
		this->list = a.list;
		return  *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add a rule
	void add_rule(const ipaddress& ip, const ipaddress& mask=ipnone);
	///////////////////////////////////////////////////////////////////////////
	/// add a rule as cstring.
	/// returns pointer to the position where it could parse the input
	const char* add_rule(const char* str);
	///////////////////////////////////////////////////////////////////////////
	/// add a rule as string
	void add_rule(const string<>& str)
	{
		this->add_rule((const char*)str);
	}
	///////////////////////////////////////////////////////////////////////////
	/// number of entries
	size_t size() const	{ return this->list.size(); }
	///////////////////////////////////////////////////////////////////////////
	/// check if ip exists in list
	bool exists(const ipaddress& ip) const
	{
		size_t pos;
		return this->list.find(ip, 0, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	/// append a rulelist
	void append(const iprulelist_t& list);

	///////////////////////////////////////////////////////////////////////////
	/// print the current rule setup to stdout
	void print(const char*prefix=NULL) const;
};



///////////////////////////////////////////////////////////////////////////////
/// ip rulelist class.
/// contains rules based on ip/mask,
/// setup is done by sequentially inserting rule strings\n
/// rule string format is:
///   - "clear" or "none" - clears all entries
///   - "all"             - allows all addesses
///   - 'ip' or 'ip/mask' or 'ip/mask bit count'
///
/// can be used inside a parameter, 
/// however, parameter entries (the rules) are executed sequencially
/// without storing them, so in contrast to other parameters,
/// if the iprulelist parameter is deleted and reinstanciated 
/// (or created after the parameter file is read)
/// the content is lost/incomplete.
class iprulelist : public iprulelist_t
{
public:

	///////////////////////////////////////////////////////////////////////////
	/// constructor.
	iprulelist()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// destructor.
	~iprulelist()
	{}
	// can use default copy/assign

	///////////////////////////////////////////////////////////////////////////
	/// assign ip rule string.
	const iprulelist& operator=(const char* str)		{ this->add_rule(str); return *this; }
	const iprulelist& operator=(const string<>& str)	{ this->add_rule(str); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// explicit compare operator.
	/// retuns always false for usage in parameters
	friend bool operator==(const iprulelist& a, const iprulelist& b)	{ return false; } 
};


///////////////////////////////////////////////////////////////////////////////
/// ip filter class.
/// contains allow/deny rules based on ip/mask,
/// setup is done by sequentially inserting rule strings\n
/// ip rule string format is:
/// - "order [mode]   - set order with [mode] beeing:
///   - "allow,deny"     - allow before deny order
///   - "deny,allow"     - deny before allow order [default]
///   - "mutual-failure" - same as deny,allow
///
/// - "allow [ip]"	  - allows an ip
/// - "deny [ip]"     - denies an ip, both with [ip] beeing:
///   - "clear" or "none" - clears all entries
///   - "all"             - allows all addesses
///   - 'ip' or 'ip/mask' or 'ip/mask bit count'
///
/// - "ddos_interval [tick]"          - interval for ddos check
/// - "ddos_max_per_interval [count]" - allowed attempts per interval
///
/// can be used inside a parameter, 
/// however, parameter entries (the rules) are executed sequencially
/// without storing them, so in contrast to other parameters,
/// if the ipfiler parameter is deleted and reinstanciated 
/// (or created after the parameter file is read)
/// the content is lost/incomplete.
class ipfilter : public global
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// order type.
	enum ord_t
	{
		ALLOWDENY=0,	///< ALLOW DENY.
						///< the allow directives are evaluated before the deny directives,
						///< hosts appearing on neither is denied.
		DENYALLOW,		///< DENY ALLOW.
						///< the deny directives are evaluated before the allow directives,
						///< hosts appearing on neither is denied. 
		MUTUALFAILURE	///< MUTUAL FAILTURE.
						///< Any host appearing on the allow list is allowed, 
						///< and any list on the deny list is denied,
						///< Any appearing on neither or both is denied (practically same as deny,allow).
	};
private:
	///////////////////////////////////////////////////////////////////////////
	/// entry in ddos list.
	/// stores tick of last access and number of previous violations
	struct access_time
	{
		ulong tick;
		uint count;
		time_t ban_until;

		access_time() : tick(0), count(0), ban_until(0)
		{}
	};
	friend struct iprulelist_t;

	///////////////////////////////////////////////////////////////////////////
	// data.
	bool (ipfilter::*mode)(const ipaddress& ip) const;	///< mode (deny,allow or allow,deny)
	iprulelist_t	allow_list;			///< list of allow rules
	iprulelist_t	deny_list;			///< list of deny rules
	smap<ipaddress, access_time>		ddos_list;			///< ddos access list
	ulong								last_tick;			///< last tick of a ddos cleanup
	ulong								ban_time;			///< time of a ddos ban
	uint								interval;			///< interval for ddos access
	uint								max_per_interval;	///< max number of violations until permanent deny
public:
	///////////////////////////////////////////////////////////////////////////
	/// constructor.
	ipfilter();

	// can use default copy/assign

	///////////////////////////////////////////////////////////////////////////
	/// check ddos and if ip is allowed.
	bool access_from(const ipaddress& ip);

	///////////////////////////////////////////////////////////////////////////
	/// check if ip is allowed.
	bool is_allowed(const ipaddress& ip) const
	{
		return (this->*mode)(ip);
	}

	///////////////////////////////////////////////////////////////////////////
	/// set compare mode.
	void set_order(ord_t m);
	
	///////////////////////////////////////////////////////////////////////////
	/// allow/deny rule function. used when allow,deny order is set
	bool allowdeny(const ipaddress& ip) const;

	///////////////////////////////////////////////////////////////////////////
	/// deny/allow rule function. used when deny,allow order is set
	bool denyallow(const ipaddress& ip) const;
	
	///////////////////////////////////////////////////////////////////////////
	/// deny setting. takes ipaddress parameters
	void deny(const ipaddress& ip, const ipaddress& mask=ipnone)
	{
		this->deny_list.add_rule(ip, mask);
	}
	///////////////////////////////////////////////////////////////////////////
	/// deny setting. takes string parameters
	void deny(const char* ipstr, const char* maskstr)
	{
		this->deny_list.add_rule(ipaddress(ipstr), ipaddress(maskstr));
	}
	void deny(const string<>& ipstr, const string<>& maskstr)
	{
		this->deny_list.add_rule(ipaddress(ipstr), ipaddress(maskstr));
	}
	///////////////////////////////////////////////////////////////////////////
	/// deny setting. takes a single string parameters
	void deny(const char* ipstr)
	{
		this->deny_list.add_rule(ipstr);
	}
	void deny(const string<>& ipstr)
	{
		this->deny_list.add_rule(ipstr);
	}
	///////////////////////////////////////////////////////////////////////////
	/// allow setting. takes ipaddress parameters
	void allow(const ipaddress& ip, const ipaddress& mask=ipnone)
	{
		this->allow_list.add_rule(ip, mask);
	}
	///////////////////////////////////////////////////////////////////////////
	/// allow setting. takes string parameters
	void allow(const char* ipstr, const char* maskstr)
	{
		this->allow_list.add_rule(ipaddress(ipstr), ipaddress(maskstr));
	}
	void allow(const string<>& ipstr, const string<>& maskstr)
	{
		this->allow_list.add_rule(ipaddress(ipstr), ipaddress(maskstr));
	}
	///////////////////////////////////////////////////////////////////////////
	/// allow setting. takes a single string parameters
	void allow(const char* str)
	{
		this->allow_list.add_rule(str);
	}
	void allow(const string<>& str)
	{
		this->allow_list.add_rule(str);
	}
	///////////////////////////////////////////////////////////////////////////
	/// assign ip rule string.
	/// rule is equivalent to httpd control strings, 
	/// does also split up multline rules
	void add_rule(const char* str);
	void add_rule(const string<> str)
	{
		this->add_rule((const char*)str);
	}

	///////////////////////////////////////////////////////////////////////////
	/// assign ip rule string.
	const ipfilter& operator=(const char* str)		{ this->add_rule(str); return *this; }
	const ipfilter& operator=(const string<>& str)	{ this->add_rule(str); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// explicit compare operator.
	/// retuns always false for usage in parameters
	friend bool operator==(const ipfilter& a, const ipfilter& b)	{ return false; } 

	///////////////////////////////////////////////////////////////////////////
	/// print the current rule setup to stdout
	void print() const;
};


NAMESPACE_END(basics)


#endif//__BASEIPFILTER__
