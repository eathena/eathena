#include "basestrsearch.h"

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
// explicit instantiation of the whole class
template class patternstring_kmp<char>;
template class patternstring_kmp<wchar_t>;


///////////////////////////////////////////////////////////////////////////////
// explicit instantiation of the whole class
template class patternstring<char>;
template class patternstring<wchar_t>;












////////////////////////////////////////////////////////////////////////////////
// taken from stlplus but removed std:: dependencies and added widechar support
// faster than a regex but different syntax and only checks for match or not
//
// WARNING: new wheel invention follows
// Given that all shells perform wildcard matching, why don't the library writers put it in the C run-time????????
// The problem:
//   *  matches any number of characters - this is achieved by matching 1 and seeing if the remainder matches
//      if not, try 2 characters and see if the remainder matches etc.
//      this must be recursive, not iterative, so that multiple *s can appear in the same wildcard expression
//   ?  matches exactly one character so doesn't need the what-if approach
//   \  escapes special characters such as *, ? and [
//   [] matches exactly one character in the set - the difficulty is the set can contain ranges, e.g [a-zA-Z0-9]
//      a set cannot be empty and the ] character can be included by making it the first character

// function for testing whether a character matches a set
// I can't remember the exact rules and I have no definitive references but:
// a set contains characters, escaped characters (I think) and ranges in the form a-z
// The character '-' can only appear at the start of the set where it is not interpreted as a range
// This is a horrible mess - blame the Unix folks for making a hash of wildcards
// first expand any ranges and remove escape characters to make life more palatable

template<typename T>
bool match_set(const T* set, const T* setend, T match)
{
	const T* ip=set;
	bool ret = true;

	if(*ip == '^')
	{	// inverts the set
		ret = false;
		ip++; set++;
	}
	for( ; ip < setend; ++ip)
	{
		switch(*ip)
		{
		// ranges
		case '-':
		{
			if( ip == set)
			{	// but on start it is a minus
				if(match==*ip)
					return ret;
			}
			else if (ip+1 == setend)
			{	// otherwise the range is not closed
				return false;
			}
			else
			{	// found a complete set
				++ip;
				// check if the char is in that range, don't care for the range order
				if( *(ip-2)<*(ip) && match>= *(ip-2) && match<= *(ip) ||	// [a-z]
					*(ip-2)>*(ip) && match<= *(ip-2) && match>= *(ip) )		// [z-a]
					return ret;		
			}
			break;
		}
		// escaped chars
		case '\\':
			ip++;
			if(ip == setend)
			{	// does not match in any case
				return false;
			}
			else if(*ip == 't')	//t -> tab
			{
				if(match=='\t')
					return ret;
			}
			else if(*ip == 'n')	//n -> return
			{
				if(match=='\n' || match=='\r')
					return ret;
			}
			else if(*ip == 's')	//s -> whitespace
			{
				if( stringcheck::isspace(match) )
					return ret;
			}
			else if(*ip == 'S')	//s -> !whitespace
			{
				if( !stringcheck::isspace(match) )
					return ret;
			}
			else if(*ip == 'd')	//s -> digit
			{
				if( stringcheck::isdigit(match) )
					return ret;
			}
			else if(*ip == 'D')	//s -> !digit
			{
				if( !stringcheck::isdigit(match) )
					return ret;
			}
			else if(*ip == '[')	//[ -> [
			{
				if( match == '[' )
					return ret;
			}
			else if(*ip == ']')	//] -> ]
			{
				if( match == '[' )
					return ret;
			}
			else if(*ip == '*')	//* -> *
			{
				if( match == '*' )
					return ret;
			}
			else if(*ip == '?')	//? -> ?
			{
				if( match == '?' )
					return ret;
			}
			break;
		// direct matches
		default:
			if(match==*ip)
				return ret;
			break;
		}
	}
	return !ret;
}
template bool match_set<char   >(const char   * set, const char   * setend, char    match);
template bool match_set<wchar_t>(const wchar_t* set, const wchar_t* setend, wchar_t match);



// the recursive bit - basically whenever a * is found you recursively call this 
// for each candidate substring match until either it succeeds or you run out of string to match
// for each * in the wildcard another level of recursion is created
template<typename T>
bool match_remainder(const T* wild, const T* wildi, const T* match, const T* matchi)
{
	//cerr << "match_remainder called at " << *matchi << " with wildcard " << *wildi << endl;
	while( *wildi && *matchi )
	{
		//cerr << "trying to match " << *matchi << " with wildcard " << *wildi << endl;
		switch(*wildi)
		{
		case '*':
		{
			++wildi;
			++matchi;
			const T* ip;
			for( ip = matchi; *ip; ++ip)
			{	// deal with * at the end of the wildcard - there is no remainder then
				if( *wildi == 0 )
				{	// run through the whole remaining string
					// and then call the recursions upwards with empty string
					if( ip[1] == 0 )
						return true;
				}
				else if( match_remainder(wild, wildi, match, ip) )
				{
					return true;
				}
			}
			return false;
		}
		case '[':
		{	// scan for the end of the set using a similar method for avoiding escaped characters
			bool found = false;
			const T* end = wildi + 1;
			for ( ; !found && *end ; ++end)
			{
				switch(*end)
				{
				case ']':
				{	// found the set, now match with its contents excluding the brackets
					if( !match_set(wildi+1, end, *matchi) )
						return false;
					found = true;
					break;
				}
				case '\\':
					if( end[1] == 0 )
						return false;
					++end;
					break;
				default:
					break;
				}
			}
			if( !found )
				return false;
			++matchi;
			wildi = end;
			break;
		}
		case '?':
			++wildi;
			++matchi;
			break;
		case '\\':
			if( wildi[1]==0 )
				return false;
			++wildi;
			if( *wildi != *matchi )
				return false;
			++wildi;
			++matchi;
			break;
		default:
			if( *wildi != *matchi )
				return false;
			++wildi;
			++matchi;
			break;
		}
	}
	bool result = (*wildi == 0) && (*matchi == 0);
	return result;
}
template bool match_remainder<char   >(const char   * wild, const char   * wildi, const char   * match, const char   * matchi);
template bool match_remainder<wchar_t>(const wchar_t* wild, const wchar_t* wildi, const wchar_t* match, const wchar_t* matchi);


// like all recursions the exported function has a simpler interface than the recursive function and is just a 'seed' to
// the recursion itself
template <typename T>
bool match_wildcard(const T* wild, const T* match)
{
	return wild && match && match_remainder<T>(wild, wild, match, match);
}
template bool match_wildcard<char   >(const char   * wild, const char   * match);
template bool match_wildcard<wchar_t>(const wchar_t* wild, const wchar_t* match);




void test_strsearch(void)
{
#if defined(DEBUG)
	uint k;
	uint elems=50000;
	ulong tick;

	string<> pattern = "aaannnaaa";
	string<> searchstring = "gcagagadgcagagaaannnaaacagagad";


	patternstring<>		ptb;
	patternstring_kmp<> ptk;

	tick = clock();
	for(k=0; k<elems; ++k)
		ptb = pattern;
	printf("booyer pattern generation  %lu (%u elems)\n", clock()-tick, elems);

	tick = clock();
	for(k=0; k<elems; ++k)
	{
		ptk = pattern;
	}
	printf("Knuth-Morris-Pratt pattern generation  %lu (%u elems)\n", clock()-tick, elems);


	tick = clock();
	size_t pos=0;
	for(k=0; k<elems; pos=0,k++)
		ptb.findnext(searchstring,pos);
	printf("booyer search %lu (%u elems)\n", clock()-tick, elems);

	tick = clock();
	for(k=0; k<elems; ++k)
	{
		ptk.findnext(searchstring);
	}
	printf("Knuth-Morris-Pratt search  %lu (%u elems)\n", clock()-tick, elems);

#endif//DEBUG
}

NAMESPACE_END(basics)
