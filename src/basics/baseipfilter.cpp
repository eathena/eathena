
#include "baseipfilter.h"
#include "baseparam.h"

NAMESPACE_BEGIN(basics)



///////////////////////////////////////////////////////////////////////////////
// iprulelist_t


///////////////////////////////////////////////////////////////////////////////
/// add a rule
void iprulelist_t::add_rule(const ipaddress& ip, const ipaddress& mask)
{
	ScopeLock sl(*this);

	size_t pos;
	const iprule_t tmp(ip,mask);
retry:
	if( this->list.find( tmp, 0, pos) )
	{	// check if new range is larger
		const iprule_t &rule = this->list[pos];
		if( ~mask > ~rule.mask )
		{	// existing rule is a subset of the new one
			// so remove it
			this->list.removeindex(pos);
			// call recursively to remove multiple matches
			//this->add_rule(ip, mask);

			// replaced the recursion with an goto
			// more ugly but does the same without the call
			// there is possibly a better solution [Karl]
			goto retry;
		}
		// otherwise keep the existing rule
		// since the new rule is only a subset of it
	}
	else
	{	// new entry
		this->list.insert( tmp );
	}
}

///////////////////////////////////////////////////////////////////////////////
/// add a rule as cstring
const char* iprulelist_t::add_rule(const char* str)
{
	ScopeLock sl(*this);
	
	while( str && *str )
	{
		for(; stringcheck::isspace(*str) || *str==',' || *str==';' || *str=='\n' || *str=='\r'; ++str) {}
		if( 0==strcasecmp(str, "all") )
		{
			str += 3;
			this->list.clear();
			this->add_rule( ipany, ipany );
		}
		else if( 0==strcasecmp(str, "none") ||
				 0==strcasecmp(str, "clear") )
		{
			str += (*str=='n'?4:5);
			this->list.clear();
		}
		else
		{	// format is "ip" or "ip/mask" or "ip/mask bit count"
			//basics::CRegExp re("(\\d+.\\d+.\\d+.\\d+)\\s*\\/\\s*(?:(\\d+.\\d+.\\d+.\\d+)|(\\d+))?");
			ipaddress addr(ipany), mask(ipnone);
			ushort port(0);
			const char* ip=ipaddress::str2ip(str,addr,mask,port);
			if( str==ip )
			{	// failed
				break;
			}
			else
			{
				if( mask == ipany )	// don't allow 0.0.0.0 masks
					mask =  ipnone;	// set them to 255.255.255.255
				this->add_rule(addr,mask);
				str=ip;
			}
		}
	}
	return str;
}

///////////////////////////////////////////////////////////////////////////////
/// append a rulelist
void iprulelist_t::append(const iprulelist_t& list)
{
	slist<iprule_t>::iterator iter(list.list);
	for(; iter; ++iter)
	{
		if( iter->addr!=ipany || iter->mask!=ipnone )
			this->list.append(*iter);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// print the current rule setup to stdout
void iprulelist_t::print(const char*prefix) const
{
	slist<iprule_t>::iterator iter(this->list);
	for(; iter; ++iter)
	{
		if(prefix) printf("%s ", prefix);
		printf("%s/", (const char*)(iter->addr));
		printf("%s\n",(const char*)(iter->mask));
	}
}



///////////////////////////////////////////////////////////////////////////////
// ipfilter

///////////////////////////////////////////////////////////////////////////////
/// constructor.
ipfilter::ipfilter() :
	mode(&ipfilter::allowdeny), 
	last_tick(0),
	ban_time(0),			// unlimited
	interval(10000),		// 10 sec
	max_per_interval(10)	// 10 tries
{}

///////////////////////////////////////////////////////////////////////////////
/// check ddos and if ip is allowed.
bool ipfilter::access_from(const ipaddress& ip)
{
	if( this->is_allowed(ip) )
	{
		const ulong curr_tick = GetTickCount();
		///////////////////////////////////////////////////
		// clean ddos list
		if( curr_tick-this->last_tick > 10*this->interval )
		{	// clean old items from ddos list
			// could possibly use a boolean bimap
			smap<ipaddress, access_time>::iterator iter(this->ddos_list);
			while(iter)
			{
				if( !iter->data.ban_until && curr_tick-iter->data.tick > this->interval )
					this->ddos_list.erase(iter->key);
				else
					++iter;
			}
			this->last_tick = curr_tick;
		}
		///////////////////////////////////////////////////
		access_time &acc = ddos_list[ip];
		const ulong prev_tick = acc.tick;
		acc.tick = curr_tick;

		if(acc.ban_until)
		{
			time_t now = time(NULL);
			if( acc.ban_until - now > 0 )
				return false;								// still banned
			else
				acc.count = acc.ban_until = 0;	
		}
		
		if( !acc.count || curr_tick-prev_tick>interval )
		{	// reset the counter
			acc.count= 1;
		}
		else
		{	// increment access counter
			if( max_per_interval < atomicincrement(&acc.count) )
			{	// violated restrictions
				if(this->ban_time)
					acc.ban_until = time(NULL)+this->ban_time;// use bantime
				else
					this->deny(ip);							// permanent ban
				return false;
			}
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// allow/deny rule function.
bool ipfilter::allowdeny(const ipaddress& ip) const
{
	if( allow_list.size()==0 || allow_list.exists(ip) )
		return !deny_list.exists(ip);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// deny/allow rule function.
bool ipfilter::denyallow(const ipaddress& ip) const
{
	if( deny_list.size()==0 || deny_list.exists(ip) )
		return allow_list.exists(ip);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// set compare mode.
void ipfilter::set_order(ord_t m)
{
	switch(m)
	{
	case ALLOWDENY:		mode = &ipfilter::allowdeny; break;
	case DENYALLOW:		mode = &ipfilter::denyallow; break;
	case MUTUALFAILURE:	mode = &ipfilter::allowdeny; break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// assign ip rule string.
void ipfilter::add_rule(const char* str)
{
	while(str && *str)
	{
		for(; stringcheck::isspace(*str) || *str==',' || *str==';' || *str=='\n' || *str=='\r'; ++str) {}
		const bool a = (0==strncasecmp(str,"allow ", 6));
		const bool b = (a)?false:(0==strncasecmp(str,"deny ", 5));
		if( a ^ b )
		{
			str+=((a)?(6):(5));
			str = (a)?this->allow_list.add_rule(str) : this->deny_list.add_rule(str);
		}
		else if( 0==strncasecmp(str, "order ", 6) )
		{
			for(str+=6; stringcheck::isspace(*str); ++str) {}

			if( 0==strncasecmp(str, "allow,deny", 10) )
			{
				str+=10;
				this->set_order(ALLOWDENY);
			}
			else if( 0==strncasecmp(str, "deny,allow", 10) )
			{
				str+=10;
				this->set_order(DENYALLOW);
			}
			else if( 0==strncasecmp(str, "mutual-failure", 14) )
			{
				str+=14;
				this->set_order(MUTUALFAILURE);
			}
		}
		else if( 0==strncasecmp(str, "ddos_interval ", 14) )
		{
			for(str+=14, this->interval=0; stringcheck::isdigit(*str); ++str)
			{
				this->interval = 10*this->interval + *str - '0';
			}
			// not smaller than 1 second
			if(this->interval<1000)
				this->interval = 1000;
			// also check the maxcount here to prevent misconfiguration
			if( this->max_per_interval>this->interval/100 )
				this->max_per_interval = this->interval/100;
		}
		else if( 0==strncasecmp(str, "ddos_max_per_interval ", 22) )
		{
			for(str+=22, this->max_per_interval=0; stringcheck::isdigit(*str); ++str)
			{
				this->max_per_interval = 10*this->max_per_interval + *str - '0';
			}
			// not smaller than 1 attempt per interval
			// and not more than 10 attempts per second
			if( this->max_per_interval<1 )
				this->max_per_interval = 1;
			else if( this->max_per_interval>this->interval/100 )
				this->max_per_interval = this->interval/100;
		}
		else if( 0==strncasecmp(str, "ddos_ban_time ", 14) )
		{
			for(str+=14, this->ban_time=0; stringcheck::isdigit(*str); ++str)
			{
				this->ban_time= 10*this->ban_time + *str - '0';
			}
			if(this->ban_time && this->ban_time < 60)
				this->ban_time = 60;		// at least a minute
			else if(this->ban_time > 2628000)
				this->ban_time = 0;			// a month is like eternity

		}
		else
		{	// failed
			break;
		}
	}
}

void ipfilter::print() const
{
	this->deny_list.print("deny");
	this->allow_list.print("allow");
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void test_ipfilter()
{
#ifdef DEBUG

	
	iprulelist ia;

	ia.add_rule("1.1.1.1");
	ia.add_rule( ipaddress("2.2.222.22"), ipaddress("255.255.255.0") );

	CParam<iprulelist> irl("iprule");
	CParam<ipfilter> ipf("ipfilter");

	CParamBase::loadFile("config1.txt");
	
	irl->print("iprule");
	ipf->print();
	
	
	ipfilter sa;

	string<> a("1.1"), b("1.1");

	sa.allow( a+'.'+b );


	sa.allow( iplocal );
	sa.allow( ipaddress("1.1.1.1") );
	sa.allow( ipaddress("1.1.1.2") );
	sa.allow( ipaddress("2.2.22.22") );
	sa.deny( ipaddress("2.2.222.22"), ipaddress("255.255.255.0") );
	sa.deny( ipaddress("2.2.22.22"), ipaddress("255.255.0.0") );

	sa = "ddos_ban_time 2";

	sa.print();

	sa.is_allowed( ipaddress("1.1.1.1") );
	sa.is_allowed( ipaddress("2.2.2.2") );
	sa.set_order(ipfilter::DENYALLOW);
	sa.is_allowed( ipaddress("2.2.2.2") );

	sa.set_order(ipfilter::ALLOWDENY);

	sa.access_from( ipaddress("1.1.1.2") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );
	sa.access_from( ipaddress("1.1.1.1") );



#endif//DEBUG
}


NAMESPACE_END(basics)
