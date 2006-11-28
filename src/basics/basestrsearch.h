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
template <class T=char> class patternstring_kmp : public string<T>
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
/// booyer-moore patternstring.
/// derived from string
/// additionally generates the pattern skip table to be used in booyer-moore search
/// fastens up continious searches of the same pattern in different strings
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class patternstring : public string<T>
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
/// simple pattern matching.
/// using *?# wildcards and character sets
///////////////////////////////////////////////////////////////////////////////
template <class T> bool match_wildcard(const T* wild, const T* match);



NAMESPACE_END(basics)


#endif//__BASESTRSEARCH_H__
