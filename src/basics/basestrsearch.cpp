#include "basestrsearch.h"


///////////////////////////////////////////////////////////////////////////////
// String search based on the Knuth-Morris-Pratt algorithm for
// linear running-time, O(m+n) where m=length of pattern and
// n=length of text.
// originally written by Andreas Magnusson in November 2001
// but was not reviews carefully enough since it was seriously broken
///////////////////////////////////////////////////////////////////////////////


// create the shift-lookup-table
template<class T> void patternstring_kmp<T>::compute_shifts(const string<T> &pattern)
{

	size_t i, next_shift = 0;
	cShifts.clear();
	cShifts.push( 0);

	// start with the second character, since the shift to the first is always 0
	for(i=1; i < pattern.length(); i++)
	{
		while(next_shift > 0 && pattern[next_shift] != pattern[i])
			next_shift = cShifts[next_shift-1];
		if(pattern[next_shift] == pattern[i])
			next_shift++;
		cShifts.push(next_shift);
	}
	cShifts[0] = 0;
}
// explicit instantiation
//template void patternstring_kmp<char   >::compute_shifts(const string<char   > &pattern);
//template void patternstring_kmp<wchar_t>::compute_shifts(const string<wchar_t> &pattern);

// search the string and return when the first occurrence is found
template<class T> int patternstring_kmp<T>::findnext(const string<T> &text) const
{
	size_t next_shift = 0;
	for(size_t i = 0; i < text.length(); i++)
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
// explicit instantiation
//template int patternstring_kmp<char   >::findnext(const string<char   > &text) const;
//template int patternstring_kmp<wchar_t>::findnext(const string<wchar_t> &text) const;

// search the string and put every occurence in a vector
template<class T> TArrayDST<size_t> patternstring_kmp<T>::findall(const string<T> &text) const
{
	size_t next_shift = 0;
	TArrayDST<size_t> positions;
	for(size_t i = 0; i < text.length(); i++)
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
// explicit instantiation
//template TArrayDST<size_t> patternstring_kmp<char   >::findall(const string<char   > &text) const;
//template TArrayDST<size_t> patternstring_kmp<wchar_t>::findall(const string<wchar_t> &text) const;

// search the string and return when the first occurrence is found
template<class T> int patternstring_kmp<T>::findnext(const string<T> &text, const string<T> &pattern)
{
	size_t next_shift = 0;
	compute_shifts(pattern);
	this->string<T>::operator=(pattern);
	for(size_t i = 0; i < text.length(); i++)
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
// explicit instantiation
//template int patternstring_kmp<char   >::findnext(const string<char   > &text, const string<char   > &pattern);
//template int patternstring_kmp<wchar_t>::findnext(const string<wchar_t> &text, const string<wchar_t> &pattern);

// search the string and put every occurence in a vector
template<class T> TArrayDST<size_t> patternstring_kmp<T>::findall(const string<T> &text, const string<T> &pattern)
{
	size_t next_shift = 0;
	TArrayDST<size_t> positions;
	compute_shifts(pattern);
	this->string<T>::operator=(pattern);
	for(size_t i = 0; i < text.length(); i++)
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
// explicit instantiation
//template TArrayDST<size_t> patternstring_kmp<char   >::findall(const string<char   > &text, const string<char   > &pattern);
//template TArrayDST<size_t> patternstring_kmp<wchar_t>::findall(const string<wchar_t> &text, const string<wchar_t> &pattern);

///////////////////////////////////////////////////////////////////////////////
// explicit instantiation of the whole class
template class patternstring_kmp<char>;
template class patternstring_kmp<wchar_t>;





///////////////////////////////////////////////////////////////////////////////
// patternstring constructor
///////////////////////////////////////////////////////////////////////////////
template<class T> void patternstring<T>::compute_shifts(const stringinterface<T> &pattern)
{
	size_t len = pattern.length();
	size_t i;
	// initialisation
	for (i=0; i<256; i++)
	{	// skip len+1 string chars if search char was not found
		// not exactly boyer-moore but fastens up the thing
		SkipTable[i] = len+1;
	}
	for (i=0; i<len; i++)
	{	// otherwise skip as only that many 
		// so the next char in the string fits with the one frome the pattern
		size_t inx = to_unsigned( pattern[i] );
		if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
			SkipTable[ inx  ] = len-i;
	}
}
// explicit instantiation
//template void patternstring<char   >::compute_shifts(const stringinterface<char   >& pattern);
//template void patternstring<wchar_t>::compute_shifts(const stringinterface<wchar_t>& pattern);

///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template<class T> bool patternstring<T>::findnext(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase) const
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
// explicit instantiation
//template bool patternstring<char   >::findnext(const stringinterface<char   >& searchstring, size_t &startpos, bool ignorecase) const;
//template bool patternstring<wchar_t>::findnext(const stringinterface<wchar_t>& searchstring, size_t &startpos, bool ignorecase) const;

///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template<class T> TArrayDST<size_t> patternstring<T>::findall(const stringinterface<T>& searchstring, bool ignorecase) const
{	// modified boyer-moore search

	// store results in this list
	TArrayDST<size_t> poslist;

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
// explicit instantiation
//template TArrayDST<size_t> patternstring<char   >::findall(const stringinterface<char   >& searchstring, bool ignorecase) const;
//template TArrayDST<size_t> patternstring<wchar_t>::findall(const stringinterface<wchar_t>& searchstring, bool ignorecase) const;


///////////////////////////////////////////////////////////////////////////////
// explicit instantiation of the whole class
template class patternstring<char>;
template class patternstring<wchar_t>;









void test_strsearch()
{

	uint k;
	uint elems=50000;
	ulong tick;

	string<> pattern = "aaannnaaa";
	string<> searchstring = "gcagagadgcagagaaannnaaacagagad";


	patternstring<>		ptb;
	patternstring_kmp<> ptk;

	tick = clock();
	for(k=0; k<elems; k++)
		ptb = pattern;
	printf("booyer pattern generation  %lu (%u elems)\n", clock()-tick, elems);

	tick = clock();
	for(k=0; k<elems; k++)
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
	for(k=0; k<elems; k++)
	{
		ptk.findnext(searchstring);
	}
	printf("Knuth-Morris-Pratt search  %lu (%u elems)\n", clock()-tick, elems);

}
