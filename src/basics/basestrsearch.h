#ifndef __BASESTRSEARCH_H__
#define __BASESTRSEARCH_H__

#include "basestring.h"
#include "basearray.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// test function
void test_strsearch(void);

///////////////////////////////////////////////////////////////////////////////
/// String search based on the Knuth-Morris-Pratt algorithm.
/// for linear running-time, O(m+n) where m=length of pattern and
/// n=length of text.
/// originally written by Andreas Magnusson in November 2001
template <typename T=char>
class patternstring_kmp : public string<T>
{
	vector<size_t>	cShifts;

	void compute_shifts(const string<T> &pattern);
public:
	patternstring_kmp()	{}
	patternstring_kmp(const string<T> &pattern) : string<T>(pattern)
	{
		compute_shifts(pattern);
	}
	const patternstring_kmp& operator=(const string<T> &pattern)
	{
		this->string<T>::operator=(pattern);
		compute_shifts(pattern);
		return *this;
	}

	int findnext(const string<T> &text) const;
	vector<size_t> findall(const string<T> &text) const;


	int findnext(const string<T> &text, const string<T> &pattern);
	vector<size_t> findall(const string<T> &text, const string<T> &pattern);
};
///////////////////////////////////////////////////////////////////////////////
// create the shift-lookup-table
template<typename T>
void patternstring_kmp<T>::compute_shifts(const string<T> &pattern)
{

	size_t i, next_shift = 0;
	cShifts.clear();
	cShifts.push( 0);

	// start with the second character, since the shift to the first is always 0
	for(i=1; i < pattern.length(); ++i)
	{
		while(next_shift > 0 && pattern[next_shift] != pattern[i])
			next_shift = cShifts[next_shift-1];
		if(pattern[next_shift] == pattern[i])
			next_shift++;
		cShifts.push(next_shift);
	}
	cShifts[0] = 0;
}
///////////////////////////////////////////////////////////////////////////////
// search the string and return when the first occurrence is found
template<typename T>
int patternstring_kmp<T>::findnext(const string<T> &text) const
{
	size_t next_shift = 0;
	for(size_t i = 0; i < text.length(); ++i)
	{
		while(next_shift > 0 && this->string<T>::operator[](next_shift) != text[i])
			next_shift = cShifts[next_shift - 1];
		
		if(this->string<T>::operator[](next_shift) == text[i])
			next_shift++;
		
		if(next_shift == this->string<T>::length())
			return i - (this->string<T>::length() - 1); // found the first so return
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
// search the string and put every occurence in a vector
template<typename T>
vector<size_t> patternstring_kmp<T>::findall(const string<T> &text) const
{
	size_t next_shift = 0;
	vector<size_t> positions;
	for(size_t i = 0; i < text.length(); ++i)
	{
		while(next_shift > 0 && this->string<T>::operator[](next_shift) != text[i])
			next_shift = cShifts[next_shift - 1];
		
		if(this->string<T>::operator[](next_shift) == text[i])
			next_shift++;
		if(next_shift == this->string<T>::length())
		{
			positions.push(i - (this->string<T>::length() - 1)); // found one, put in list
			next_shift = cShifts[next_shift - 1]; // restart pattern with last shift
		}
	}
	return positions;
}
///////////////////////////////////////////////////////////////////////////////
// search the string and return when the first occurrence is found
template<typename T>
int patternstring_kmp<T>::findnext(const string<T> &text, const string<T> &pattern)
{
	size_t next_shift = 0;
	compute_shifts(pattern);
	this->string<T>::operator=(pattern);
	for(size_t i = 0; i < text.length(); ++i)
	{
		while(next_shift > 0 && pattern[next_shift] != text[i])
			next_shift = cShifts[next_shift - 1];
		
		if(pattern[next_shift] == text[i])
			next_shift++;
		
		if(next_shift == pattern.length())
			return i - (pattern.length() - 1); // found the first so return
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
// search the string and put every occurence in a vector
template<typename T>
vector<size_t> patternstring_kmp<T>::findall(const string<T> &text, const string<T> &pattern)
{
	size_t next_shift = 0;
	vector<size_t> positions;
	compute_shifts(pattern);
	this->string<T>::operator=(pattern);
	for(size_t i = 0; i < text.length(); ++i)
	{
		while(next_shift > 0 && pattern[next_shift] != text[i])
			next_shift = cShifts[next_shift - 1];
		
		if(pattern[next_shift] == text[i])
			next_shift++;
		if(next_shift == pattern.length())
		{
			positions.push(i - (pattern.length() - 1)); // found one, put in list
			next_shift = cShifts[next_shift - 1]; // restart pattern with last shift
		}
	}
	return positions;
}




///////////////////////////////////////////////////////////////////////////////
/// booyer-moore patternstring.
/// derived from string
/// additionally generates the pattern skip table to be used in booyer-moore search
/// fastens up continious searches of the same pattern in different strings
template <typename T=char>
class patternstring : public string<T>
{
	friend class string<T>;
	// table size is bound to 8bit values
	size_t	SkipTable[256];
	void compute_shifts(const stringinterface<T> &pattern);
public:
	patternstring()	{}
	patternstring(const string<T>& pattern) : string<T>(pattern)
	{
		compute_shifts(pattern);
	}
	const patternstring& operator=(const string<T> &pattern)
	{
		this->string<T>::operator=(pattern);
		compute_shifts(pattern);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////////
	/// Search function
	///////////////////////////////////////////////////////////////////////////////
	bool findnext(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase=false) const;
	vector<size_t> findall(const stringinterface<T>& searchstring, bool ignorecase=false) const;
};

///////////////////////////////////////////////////////////////////////////////
// patternstring constructor
template<typename T>
void patternstring<T>::compute_shifts(const stringinterface<T> &pattern)
{
	size_t len = pattern.length();
	size_t i;
	// initialisation
	for (i=0; i<256; ++i)
	{	// skip len+1 string chars if search char was not found
		// not exactly boyer-moore but fastens up the thing
		SkipTable[i] = len+1;
	}
	for (i=0; i<len; ++i)
	{	// otherwise skip as only that many 
		// so the next char in the string fits with the one from the pattern
		size_t inx = to_unsigned( pattern[i] );
		if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
			SkipTable[ inx  ] = len-i;
	}
}
///////////////////////////////////////////////////////////////////////////////
// Search function
template<typename T>
bool patternstring<T>::findnext(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase) const
{	// modified boyer-moore search
	
	size_t i,k,sp;
	size_t len = this->length();

	sp=i=startpos;
	k=0; 
	while( i<searchstring.length() && k<len )
	{
		if( ignorecase ? 
			( locase(searchstring[i]) != locase((*this)[k]) ) :
			( searchstring[i] != (*this)[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( searchstring[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}
	}
	if( k<len ) // not found
		return false;
	startpos = sp;
	return true;
}
///////////////////////////////////////////////////////////////////////////////
// Search function
template<typename T>
vector<size_t> patternstring<T>::findall(const stringinterface<T>& searchstring, bool ignorecase) const
{	// modified boyer-moore search

	// store results in this list
	vector<size_t> poslist;

	size_t i,k,sp;
	size_t len = this->length();

	sp=i=0;
	k=0; 
	while( i<searchstring.length() )
	{
		if( ignorecase ? 
			( locase(searchstring[i]) != locase((*this)[k]) ) :
			( searchstring[i] != (*this)[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( searchstring[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}

		if(k>=len)
		{	// found
			poslist.push(sp);
			sp++;
		}
	}
	return poslist;
}




///////////////////////////////////////////////////////////////////////////////
/// simple pattern matching.
/// using *?# wildcards and character sets
///////////////////////////////////////////////////////////////////////////////
template <typename T>
bool match_wildcard(const T* wild, const T* match);
template <typename T>
bool match_wildcard(const T* wild, const string<T>& match)
{
	return match_wildcard<T>(wild, match.c_str());
}
template <typename T>
bool match_wildcard(const string<T>& wild, const T* match)
{
	return match_wildcard<T>(wild.c_str(), match);
}
template <typename T>
bool match_wildcard(const string<T>& wild, const string<T>& match)
{
	return match_wildcard<T>(wild.c_str(), match.c_str());
}

NAMESPACE_END(basics)


#endif//__BASESTRSEARCH_H__
