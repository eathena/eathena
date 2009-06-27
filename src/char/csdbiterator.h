// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CSDBITERATOR_H_
#define _CSDBITERATOR_H_

#include "../common/cbasetypes.h" // bool


typedef struct CSDBIterator CSDBIterator;


/// CharServer iterator base class interface
struct CSDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(CSDBIterator* self);

	/// Fetches the next entry's key.
	/// @param self Iterator
	/// @param key an entry's key
	/// @return true if successful
	bool (*next)(CSDBIterator* self, int* key);
};


#endif // _CSDBITERATOR_H_
