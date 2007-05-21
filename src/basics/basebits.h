#ifndef __BASEBITS_H__
#define __BASEBITS_H__

#include "basetypes.h"


//////////////////////////////////////////////////////////////////////////
// bit twiddling
// derived from various sources
//////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
/// Check the byte-order of the CPU.
const int LSB_FIRST = 0;
const int MSB_FIRST = 1;
extern inline int CheckByteOrder(void)
{
    static short  w = 0x0001;
    static char  *b = (char *) &w;
    return(b[0] ? LSB_FIRST : MSB_FIRST);
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
extern inline uchar GetByte(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (uchar)(val      );
	case 1:
		return (uchar)(val>>0x08);
	case 2:
		return (uchar)(val>>0x10);
	case 3:
		return (uchar)(val>>0x18);
	default:
		return 0;	//better throw something here
	}
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
extern inline ushort GetWord(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (ushort)(val      );
	case 1:
		return (ushort)(val>>0x10);
	default:
		return 0;	//better throw something here
	}	
}
extern inline ushort GetHighWord(uint32 val)
{
	return (ushort)(val>>0x10);
}
extern inline ushort GetLowWord(uint32 val)
{
	return (ushort)(val      );
}

//////////////////////////////////////////////////////////////////////////
/// dword access from 64bits (input both signed and unsigned)
extern inline ulong GetHighDWord(uint64 val)
{
	return (ulong)(val>>0x20);
}
extern inline ulong GetLowDWord(uint64 val)
{
	return (ulong)(val      );
}
extern inline ulong GetHighDWord(sint64 val)
{
	return (ulong)(val>>0x20);
}
extern inline ulong GetLowDWord(sint64 val)
{
	return (ulong)(val      );
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
extern inline ushort MakeWord(uchar byte0, uchar byte1)
{
	return (ushort)(
			  (((ushort)byte0)      )
			| (((ushort)byte1)<<0x08)
			);
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
extern inline uint32 MakeDWord(ushort word0, ushort word1)
{
	return 	  (((uint32)word0)      )
			| (((uint32)word1)<<0x10);
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
extern inline uint32 MakeDWord(uchar byte0, uchar byte1, uchar byte2, uchar byte3)
{
	return 	  (((uint32)byte0)      )
			| (((uint32)byte1)<<0x08)
			| (((uint32)byte2)<<0x10)
			| (((uint32)byte3)<<0x18);
}

//////////////////////////////////////////////////////////////////////////
/// Swap two bytes in a byte stream
extern inline void SwapTwoBytes(char *p)
{	if(p)
	{	char tmp =p[0];
		p[0] = p[1];
		p[1] = tmp;
	}
}

//////////////////////////////////////////////////////////////////////////
/// Swap the bytes within a 16-bit WORD.
extern inline ushort SwapTwoBytes(ushort w)
{
    return	 (ushort)(
			  ((w & 0x00FF) << 0x08)
			| ((w & 0xFF00) >> 0x08)
			);
}

//////////////////////////////////////////////////////////////////////////
/// Swap the 4 bytes in a byte stream
extern inline void SwapFourBytes(char *p)
{	if(p)
	{	char tmp;
		tmp  = p[0];
		p[0] = p[3];
		p[3] = tmp;
		tmp  = p[1];
		p[1] = p[2];
		p[2] = tmp;
	}
}

//////////////////////////////////////////////////////////////////////////
/// Swap the 4 bytes within a 32-bit DWORD.
extern inline uint32 SwapFourBytes(uint32 w)
{
    return	  ((w & 0x000000FF) << 0x18)
			| ((w & 0x0000FF00) << 0x08)
			| ((w & 0x00FF0000) >> 0x08)
			| ((w & 0xFF000000) >> 0x18);
}

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an N-bit integer in O(lg(N)) operations
// in this case for 32bit input it would be 11 operations
#ifdef log2 //glibc defines this as macro
#undef log2
#endif
inline unsigned long log2(unsigned long v)
{
//	static const unsigned long b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
//	static const unsigned long S[] = {1, 2, 4, 8, 16};
	// result of log2(v) will go here
	register uint32 c = 0; 
//	int i;
//	for (i = 4; i >= 0; i--) 
//	{
//	  if (v & b[i])
//	  {
//		v >>= S[i];
//		c |= S[i];
//	  } 
//	}
	// unroll for speed...
//	if (v & b[4]) { v >>= S[4]; c |= S[4]; } 
//	if (v & b[3]) { v >>= S[3]; c |= S[3]; }
//	if (v & b[2]) { v >>= S[2]; c |= S[2]; }
//	if (v & b[1]) { v >>= S[1]; c |= S[1]; }
//	if (v & b[0]) { v >>= S[0]; c |= S[0]; }
	// put values in for more speed...
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	if (v & ULLCONST(0xFFFFFFFF00000000)) { v >>= 0x20; c |= 0x20; } 
#endif
	if (v & 0xFFFF0000) { v >>= 0x10; c |= 0x10; } 
	if (v & 0x0000FF00) { v >>= 0x08; c |= 0x08; }
	if (v & 0x000000F0) { v >>= 0x04; c |= 0x04; }
	if (v & 0x0000000C) { v >>= 0x02; c |= 0x02; }
	if (v & 0x00000002) { v >>= 0x01; c |= 0x01; }

	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an N-bit integer.
/// if you know it is a power of 2
inline unsigned long log2_(unsigned long v)
{
//	const unsigned long b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
//	register ulong c = ((v & b[0]) != 0);
//	int i;
//	for (i = 4; i >= 1; i--) 
//	{
//	  c |= ((v & b[i]) != 0) << i;
//	}
	// unroll for speed...
//	c |= ((v & b[4]) != 0) << 4;
//	c |= ((v & b[3]) != 0) << 3;
//	c |= ((v & b[2]) != 0) << 2;
//	c |= ((v & b[1]) != 0) << 1;
//	c |= ((v & b[0]) != 0) << 0;
	// unroll for speed...
	// put values in for more speed...
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	register ulong c = ((v & ULLCONST(0xAAAAAAAAAAAAAAAA)) != 0);
	c |= ((v & ULLCONST(0xFFFFFFFF00000000)) != 0) << 5;
	c |= ((v & ULLCONST(0xFFFF0000FFFF0000)) != 0) << 4;
	c |= ((v & ULLCONST(0xFF00FF00FF00FF00)) != 0) << 3;
	c |= ((v & ULLCONST(0xF0F0F0F0F0F0F0F0)) != 0) << 2;
	c |= ((v & ULLCONST(0xCCCCCCCCCCCCCCCC)) != 0) << 1;
	c |= ((v & ULLCONST(0xAAAAAAAAAAAAAAAA)) != 0) << 0;
#else
	register ulong c = ((v & 0xAAAAAAAA) != 0);
	c |= ((v & 0xFFFF0000) != 0) << 4;
	c |= ((v & 0xFF00FF00) != 0) << 3;
	c |= ((v & 0xF0F0F0F0) != 0) << 2;
	c |= ((v & 0xCCCCCCCC) != 0) << 1;
	c |= ((v & 0xAAAAAAAA) != 0) << 0;
#endif
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an integer with a lookup table.
/// The lookup table method takes only about 7 operations 
/// to find the log of a 32-bit value. 
/// extended for 64-bit quantities, it would take roughly 9 operations.
extern inline unsigned long log2t(unsigned long v)
{
	static const unsigned char LogTable256[] = 
	{
	  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	register unsigned long c; // c will be lg(v)
	register unsigned long t;
	register unsigned long tt;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	register unsigned long ttt = (v >> 32);
	if(ttt)
	{
		tt = (ttt >> 16);
		if(tt)
		{
			t = v >> 24;
			c = (t) ? 32+24 + LogTable256[t] : 32+16 + LogTable256[tt & 0xFF];
		}
		else 
		{
			t = v & 0xFF00;
			c = (t) ? 32+8 + LogTable256[t >> 8] : 32+LogTable256[v & 0xFF];
		}
	}
	else 
#endif
	{
		tt = (v >> 16);
		if(tt)
		{
			t = v >> 24;
			c = (t) ? 24 + LogTable256[t] : 16 + LogTable256[tt & 0xFF];
		}
		else 
		{
			t = v & 0xFF00;
			c = (t) ? 8 + LogTable256[t >> 8] : LogTable256[v & 0xFF];
		}
	}

	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, in parallel.
inline unsigned long bit_count(unsigned long v)
{
//	static const ulong S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
//	static const ulong B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
//	v = ((v >> S[0]) & B[0]) + (v & B[0]);
//	v = ((v >> S[1]) & B[1]) + (v & B[1]);
//	v = ((v >> S[2]) & B[2]) + (v & B[2]);
//	v = ((v >> S[3]) & B[3]) + (v & B[3]);
//	v = ((v >> S[4]) & B[4]) + (v & B[4]);
	// put values in
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v = ((v >> 0x01) & ULLCONST(0x5555555555555555)) + (v & ULLCONST(0x5555555555555555));
	v = ((v >> 0x02) & ULLCONST(0x3333333333333333)) + (v & ULLCONST(0x3333333333333333));
	v = ((v >> 0x04) & ULLCONST(0x0F0F0F0F0F0F0F0F)) + (v & ULLCONST(0x0F0F0F0F0F0F0F0F));
	v = ((v >> 0x08) & ULLCONST(0x00FF00FF00FF00FF)) + (v & ULLCONST(0x00FF00FF00FF00FF));
	v = ((v >> 0x10) & ULLCONST(0x0000FFFF0000FFFF)) + (v & ULLCONST(0x0000FFFF0000FFFF));
	v = ((v >> 0x20) & ULLCONST(0x00000000FFFFFFFF)) + (v & ULLCONST(0x00000000FFFFFFFF));
#else
	v = ((v >> 0x01) & 0x55555555) + (v & 0x55555555);
	v = ((v >> 0x02) & 0x33333333) + (v & 0x33333333);
	v = ((v >> 0x04) & 0x0F0F0F0F) + (v & 0x0F0F0F0F);
	v = ((v >> 0x08) & 0x00FF00FF) + (v & 0x00FF00FF);
	v = ((v >> 0x10) & 0x0000FFFF) + (v & 0x0000FFFF);
#endif
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, with table.
inline unsigned long bit_count_t(unsigned long v)
{
	// Counting bits set by lookup table 
	static const unsigned char BitsSetTable256[] = 
	{
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};
	unsigned long c=0; // c is the total bits set in v
	while(v)
	{
		c += BitsSetTable256[v&0xff];
		v >>= NBBY;
	}
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, Brian Kernighan's way.
inline unsigned long bit_count_k(unsigned long v)
{
	unsigned long c; // c accumulates the total bits set in v
	for (c = 0; v; ++c)
	{	// clear the least significant bit set
		v &= v - 1; 
	}
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 12-bit words using 64-bit instructions.
inline unsigned long bit_count_12_i64(unsigned long v)
{	
	unsigned long c; // c accumulates the total bits set in v
	// option 1, for at most 12-bit values in v:
	c = (v * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f;
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 24-bit words using 64-bit instructions.
inline unsigned long bit_count_24_i64(unsigned long v)
{	// option 2, for at most 24-bit values in v:
	unsigned long c; // c accumulates the total bits set in v
	c =  (v & 0xfff) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421);
	c += ((v & 0xfff000) >> 12) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421);
	c %= 0x1f;
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 32-bit words using 64-bit instructions.
inline unsigned long bit_count_32_i64(unsigned long v)
{	// option 3, for at most 32-bit values in v:
	unsigned long c; // c accumulates the total bits set in v
	c = (((v & 0xfff) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) +
		((v & 0xfff000) >> 12) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f; 
	c += ((v >> 24) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f; 
	// This method requires a 64-bit CPU with fast modulus division to be efficient. 
	// The first option takes only 6 operations; the second option takes 13; 
	// and the third option takes 17. 
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// calculate parity in parallel.
inline bool parity(unsigned long v)
{	// The method above takes around 9 operations for 32-bit words. 
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v ^= v >> 32;
#endif
	v ^= v >> 16;
	v ^= v >> 8;
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}

//////////////////////////////////////////////////////////////////////////
/// calculate parity using Brian Kernigan's bit counting.
inline bool parity_k(unsigned long v)
{	// using Brian Kernigan's bit counting
	// The time it takes is proportional to the number of bits set. 
	bool parity = false;  // parity will be the parity of b
	while (v)
	{
		parity = !parity;
		v = v & (v - 1);
	}
	return parity;
}

//////////////////////////////////////////////////////////////////////////
/// Compute parity of a byte using 64-bit multiply and modulus division.
/// The method takes around 7 operations, but only works on bytes. 
inline bool parity_b(unsigned char v)
{	
	return (((v * ULLCONST(0x0101010101010101)) & ULLCONST(0x8040201008040201)) % 0x1FF) & 1;
}

//////////////////////////////////////////////////////////////////////////
/// Compute parity with table.
inline bool parity_t(unsigned long v)
{
	//Thanks to Mathew Hendry for pointing out the shift-lookup idea at 
	// the end on Dec. 15, 2002. That optimization shaves two operations off 
	// using only shifting and XORing to find the parity.
	// Compute parity by lookup table 
	static const bool ParityTable[] = 
	{
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
	};
	bool parity=false;
	while(v)
	{
		parity ^= ParityTable[(v & 0xff)];
		v >>= NBBY;
	}
	return parity;
}

//////////////////////////////////////////////////////////////////////////
/// Reverse the bits in a byte with 7 operations.
inline uchar bit_reverse(uchar b)
{
	return (uchar)(((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
}

//////////////////////////////////////////////////////////////////////////
/// Reverse the bits in a byte with 64bit instructions.
inline uchar bit_reverse_i64(uchar b)
{	// The multiply operation creates five separate copies of the 8-bit byte 
	// pattern to fan-out into a 64-bit value. The and operation selects the bits that 
	// are in the correct (reversed) positions, relative to each 10-bit groups of bits. 
	// The multiply and the and operations copy the bits from the original byte 
	// so they each appear in only one of the 10-bit sets. 
	// The reversed positions of the bits from the original byte coincide with 
	// their relative positions within any 10-bit set. 
	// The last step, which involves modulus division by 2^10 - 1, 
	// has the effect of merging together each set of 10 bits 
	// (from positions 0-9, 10-19, 20-29, ...) in the 64-bit value. 
	// They do not overlap, so the addition steps underlying the modulus division 
	// behave like or operations. 
	return (b * ULLCONST(0x0202020202) & ULLCONST(0x010884422010)) % 1023;
}

//////////////////////////////////////////////////////////////////////////
/// Reverse an N-bit quantity in parallel in 5 * lg(N) operations.
/// This method is best suited to situations where N is large.
/// Any reasonable optimizing C compiler should treat the dereferences 
/// of B, ~B, and S as constants, requiring no evaluation other than perhaps 
/// a load operation for some of the B and ~B references.
/// See Dr. Dobb's Journal 1983, Edwin Freed's article on Binary Magic Numbers 
/// for more information. 
/// Anyway I would not count on that, so I put values explicitely.
inline unsigned long bit_reverse(unsigned long v)
{
//	static const ulong S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
//	static const ulong B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
//	v = ((v >> S[0]) & B[0]) | ((v << S[0]) & ~B[0]); // swap odd and even bits
//	v = ((v >> S[1]) & B[1]) | ((v << S[1]) & ~B[1]); // swap consecutive pairs
//	v = ((v >> S[2]) & B[2]) | ((v << S[2]) & ~B[2]); // swap nibbles ...
//	v = ((v >> S[3]) & B[3]) | ((v << S[3]) & ~B[3]);
//	v = ((v >> S[4]) & B[4]) | ((v << S[4]) & ~B[4]);
	// better set it by hand
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v = ((v >> 0x01) & ULLCONST(0x5555555555555555)) | ((v << 0x01) & ~ULLCONST(0x5555555555555555));
	v = ((v >> 0x02) & ULLCONST(0x3333333333333333)) | ((v << 0x02) & ~ULLCONST(0x3333333333333333));
	v = ((v >> 0x04) & ULLCONST(0x0F0F0F0F0F0F0F0F)) | ((v << 0x04) & ~ULLCONST(0x0F0F0F0F0F0F0F0F));
	v = ((v >> 0x08) & ULLCONST(0x00FF00FF00FF00FF)) | ((v << 0x08) & ~ULLCONST(0x00FF00FF00FF00FF));
	v = ((v >> 0x10) & ULLCONST(0x0000FFFF0000FFFF)) | ((v << 0x10) & ~ULLCONST(0x0000FFFF0000FFFF));
	v = ((v >> 0x20) & ULLCONST(0x00000000FFFFFFFF)) | ((v << 0x20) & ~ULLCONST(0x00000000FFFFFFFF));
#else
	v = ((v >> 0x01) & 0x55555555) | ((v << 0x01) & ~0x55555555);
	v = ((v >> 0x02) & 0x33333333) | ((v << 0x02) & ~0x33333333);
	v = ((v >> 0x04) & 0x0F0F0F0F) | ((v << 0x04) & ~0x0F0F0F0F);
	v = ((v >> 0x08) & 0x00FF00FF) | ((v << 0x08) & ~0x00FF00FF);
	v = ((v >> 0x10) & 0x0000FFFF) | ((v << 0x10) & ~0x0000FFFF);
#endif
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// check if a number is power of 2.
/// roughly; if one (and only one) bit is set
extern inline bool isPowerOf2(unsigned long i)
{
	//return (i & (i - 1)) == 0; 
	// with drawback that 0 is incorrectly considered a power of 2
	// therefore:
	//return (i > 0) && (0==(i & (i - 1)));
	// or more short:
	return i && !(i & (i - 1));
}

//////////////////////////////////////////////////////////////////////////
/// round up to the next power of 2.
extern inline unsigned long RoundPowerOf2(unsigned long v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v |= v >> 32;
#endif
	v++;
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// Compute modulus division by 1 << s without a division operator.
extern inline unsigned long moduloPowerOf2(unsigned long v, unsigned long s)
{	// Most programmers learn this trick early, but it was included for the 
	// sake of completeness. 
	return v & ( (1ul<<s) - 1); // v % 2^s
}

//////////////////////////////////////////////////////////////////////////
/// Compute modulus division by (1 << s) - 1 without a division operator.
extern inline unsigned long moduloPowerOf2_1(unsigned long v, unsigned long s)
{	// This method of modulus division by an integer that is one less than 
	// a power of 2 takes at most 5 + (4 + 5 * ceil(N / s)) * ceil(lg(N / s)) operations, 
	// where N is the number of bits in the numerator. 
	// In other words, it takes at most O(N * lg(N)) time. 
	const unsigned long d = (1 << s) - 1; // so d is either 1, 3, 7, 15, 31, ...).
	unsigned long m;                      // n % d goes here.
	for (m=v; v>d; v=m)
	for (m=0; v; v >>= s)
	{
		m += v & d;
	}
	// Now m is a value from 0 to d, but since with modulus division
	// we want m to be 0 when it is d.
	m = (m==d) ? 0 : m; // or: ((m + 1) & d) - 1;
	return m;
}

//////////////////////////////////////////////////////////////////////////
/// Determine if a word has a zero byte.
inline bool has_zeros(unsigned long v)
{	// check if any 8-bit byte in it is 0
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	return (bool)(~((((v & ULLCONST(0x7F7F7F7F7F7F7F7F)) + ULLCONST(0x7F7F7F7F7F7F7F7F)) | v) | ULLCONST(0x7F7F7F7F7F7F7F7F)));
#else
	return (bool)(~((((v & 0x7F7F7F7F) + 0x7F7F7F7F) | v) | 0x7F7F7F7F));
#endif
	// The code works by first zeroing the high bits of the 4 bytes in the word. 
	// Next, it adds a number that will result in an overflow to the high bit of 
	// a byte if any of the low bits were initialy set. 
	// Next the high bits of the original word are ORed with these values; 
	// thus, the high bit of a byte is set if any bit in the byte was set. 
	// Finally, we determine if any of these high bits are zero 
	// by ORing with ones everywhere except the high bits and inverting the result. 

	// The code above may be useful when doing a fast string copy 
	// in which a word is copied at a time; 
	// it uses at most 6 operations (and at least 5). 
	// On the other hand, testing for a null byte in the obvious ways 
	// have at least 7 operations (when counted in the most sparing way), 
	// and at most 12:
	// bool hasNoZeroByte = ((v & 0xff) && (v & 0xff00) && (v & 0xff0000) && (v & 0xff000000))
	// or:
	// unsigned char * p = (unsigned char *) &v;  
	// bool hasNoZeroByte = *p && *(p + 1) && *(p + 2) && *(p + 3);
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) using binary magics.
/// are useful for linearizing 2D integer coordinates, 
/// so x and y are combined into a single number that can be compared 
/// easily and has the property that a number is usually close to another 
/// if their x and y values are close. 
inline unsigned long interleave(unsigned long x, unsigned long y)
{	// Interleave bits by Binary Magic Numbers 
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	const unsigned long B[] = {ULLCONST(0x5555555555555555), ULLCONST(0x3333333333333333), ULLCONST(0x0F0F0F0F0F0F0F0F), ULLCONST(0x00FF00FF00FF00FF), ULLCONST(0x0000FFFF0000FFFF)};
	const unsigned long S[] = {1, 2, 4, 8, 16};
	x = ((x | (x << S[4])) & B[4]);
	x = ((x | (x << S[3])) & B[3]);
	x = ((x | (x << S[2])) & B[2]);
	x = ((x | (x << S[1])) & B[1]);
	x = ((x | (x << S[0])) & B[0]);
	y = ((y | (y << S[4])) & B[4]);
	y = ((y | (y << S[3])) & B[3]);
	y = ((y | (y << S[2])) & B[2]);
	y = ((y | (y << S[1])) & B[1]);
	y = ((y | (y << S[0])) & B[0]);
#else
	const unsigned long B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
	const unsigned long S[] = {1, 2, 4, 8};
	x = ((x | (x << S[3])) & B[3]);
	x = ((x | (x << S[2])) & B[2]);
	x = ((x | (x << S[1])) & B[1]);
	x = ((x | (x << S[0])) & B[0]);
	y = ((y | (y << S[3])) & B[3]);
	y = ((y | (y << S[2])) & B[2]);
	y = ((y | (y << S[1])) & B[1]);
	y = ((y | (y << S[0])) & B[0]);
#endif
	return x | (y << 1);
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) the obvious way.
inline unsigned long interleave_trivial(unsigned short x, unsigned short y)
{	// bits of x are in the even positions and y in the odd;
	unsigned int z = 0; // z gets the resulting 32-bit Morton Number.
	size_t i;
	for(i=0; i<sizeof(x)*NBBY; ++i)// unroll for more speed...
	{
		z |= (x & 1ul << i) << i | (y & 1ul << i) << (i + 1);
	}
	return z;
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) using a table.
inline unsigned long interleave_t(unsigned short x, unsigned short y)
{
	static const unsigned short MortonTable256[] = 
	{
		0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 
		0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, 
		0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 
		0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, 
		0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 
		0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, 
		0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
		0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
		0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
		0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
		0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
		0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
		0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
		0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
		0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
		0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
		0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
		0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
		0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
		0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
		0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
		0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
		0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
		0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
		0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
		0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
		0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
		0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
		0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 
		0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
		0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
		0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
	};
	unsigned long z;   // gets the resulting 32-bit Morton Number.
	z = (MortonTable256[y & 0xFF] << 1) | MortonTable256[x & 0xFF] |
		(MortonTable256[y >> 8] << 1 | MortonTable256[x >> 8]) << 16;


	z = (MortonTable256[y & 0xFF] << 1) | MortonTable256[x & 0xFF] |
		(MortonTable256[y >> 8] << 1 | MortonTable256[x >> 8]) << 16;
	return z;
}

//////////////////////////////////////////////////////////////////////////
/// square root with newton approximation.
/// starting condition calculated with two approximations
/// sqrt(n) = n^1/2 = n / n^1/2 = n / 2^log2(n^1/2) = n / 2^(log2(n)/2)
/// and calculating a/2^b with left shift as a>>b
/// which results in a larger value than necessary 
/// because the integer log2 returns the floored logarism and is smaller than the real log2
/// second approximation is
/// sqrt(n) = n^1/2 = 2^(log2(n)/2) which is calculated as 1<<(log2(n)/2)
/// resulting in a value smaller than necessary because of the integer log2 
/// calculation the mean of those two approximations gets closer to the real value, 
/// only slightly faster than the buildin double sqrt and therefore useless
#ifdef isqrt
#undef isqrt
#endif
template<typename T>
inline T isqrt(const T& n)
{
	if(n>0)
	{
		T q=0, xx = (log2(n)/2), qx = ((n>>xx) + (1ul<<xx))/2;
		do
		{
			q  = qx;
			qx = (q + n/q)/2;
		}
		while( q!=qx && q+1!=qx );
		return q;
	}
	// should set matherr or throw something when negative
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/// The expression evaluates ie. sign = v >> 31 for 32-bit integers. 
/// This is one operation faster than the obvious way, 
/// sign = -(v > 0). This trick works because when integers are shifted right, 
/// the value of the far left bit is copied to the other bits. 
/// The far left bit is 1 when the value is negative and 0 otherwise; 
template<typename T>
inline T sign(const T& v)
{
	T sign;   // the result goes here 

	// if v < 0 then -1, else 0
	//sign = v >> (sizeof(T) * NBBY - 1); 

	// if v < 0 then -1, else +1
	//sign = +1 | (v >> (sizeof(int) * 8 - 1));
	
	// Alternatively, for -1, 0, or +1
	sign = (v != 0) | (v >> (sizeof(int) * 8 - 1));  // -1, 0, or +1
	return sign;
	// Caveat: On March 7, 2003, Angus Duggan pointed out that the 1989 ANSI C 
	// specification leaves the result of signed right-shift implementation-defined, 
	// so on some systems this hack might not work. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the integer absolute value (abs) without branching.
template<typename T>
inline T iabs(const T& v)
{	
	return (+1 | (v >> (sizeof(T) * NBBY - 1))) * v;
	// Some CPUs don't have an integer absolute value instruction 
	// (or the compiler fails to use them). On machines where branching 
	// is expensive, the above expression can be faster than the obvious 
	// approach, r = (v < 0) ? -v : v, even though the number of operations 
	// is the same. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the minimum (min) of two integers without branching.
template<typename T>
inline T imin(const T& x, const T& y)
{	
	return y + ((x - y) & ((x - y) >> (sizeof(T) * NBBY - 1))); // min(x, y)
	// On machines where branching is expensive, the above expression can be faster 
	// than the obvious approach, r = (x < y) ? x : y, 
	// even though it involves one more instruction. 
	// (Notice that (x - y) only needs to be evaluated once.) It works because 
	// if x < y, then (x - y) >> 31 will be all ones (on a 32-bit integer machine), 
	// so r = y + (x - y) & ~0 = y + x - y = x. 
	// Otherwise, if x >= y, then (x - y) >> 31 will be all zeros, 
	// so r = y + (x - y) & 0 = y. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the maximum (max) of two integers without branching.
template<typename T>
inline T imax(const T& x, const T& y)
{	
	return  x - ((x - y) & ((x - y) >> (sizeof(int) * NBBY - 1))); // max(x, y)
	// On machines where branching is expensive, 
	// the above expression can be faster than the obvious approach, 
	// r = (x < y) ? x : y, even though it involves one more instruction. 
	// (Notice that (x - y) only needs to be evaluated once.) It works because 
	// if x < y, then (x - y) >> 31 will be all ones (on a 32-bit integer machine), 
	// so r = y + (x - y) & ~0 = y + x - y = x. 
	// Otherwise, if x >= y, then (x - y) >> 31 will be all zeros, 
	// so r = y + (x - y) & 0 = y. 
}

//////////////////////////////////////////////////////////////////////////
/// calculate pow n on base 2.
#ifdef pow2 // just to be sure
#undef pow2
#endif
extern inline unsigned long pow2(unsigned long v)
{
	if( v < NBBY*sizeof(unsigned long) )
		return 1ul<<v;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/// calculate 10 to the power of exp using a integer operations with table.
#ifdef pow10
#undef pow10
#endif
inline uint64 pow10(uint exp)
{
	static const uint64 table[] = { ULLCONST(100000000), ULLCONST(10), ULLCONST(100), ULLCONST(1000), 
									ULLCONST(10000), ULLCONST(100000) , ULLCONST(1000000), ULLCONST(10000000)};
	if(!exp)
		return 1;
	else if(exp<8)
		return table[exp&0x7];
	else if(exp>19)
		return UINT64_MAX;

	uint64 res=table[0];
	exp -= 8;
	while(exp)
	{
		if(exp<8)
		{
			res *= table[exp&0x7];
			break;
		}
		else
		{
			res*=table[0];
			exp -= 8;
		}
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
// Multiplication of two 64-bit integers, giving a 128-bit result.
// Algorithm M in Knuth section 4.3.1, with the loop hand-unrolled.
inline void mult64(const uint64 u, const uint64 v, uint64& high, uint64& low)
{
	const uint64 low_mask = LLCONST(0xffffffff);
	const uint64 u0 = u & low_mask;
	const uint64 u1 = u >> 32;
	const uint64 v0 = v & low_mask;
	const uint64 v1 = v >> 32;
	uint64 t = u0 * v0;
	low = t & low_mask;
	t = u1 * v0 + (t >> 32);
	uint64 w1 = t & low_mask;
	uint64 w2 = t >> 32;
	uint64 x = u0 * v1 + w1;
	low += (x & low_mask) << 32;
	high = u1 * v1 + w2 + (x >> 32);
}

NAMESPACE_END(basics)

#endif//__BASEBITS_H__
