// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOTKEYDB_H_
#define _HOTKEYDB_H_

#include "../common/mmo.h" // struct hotkey, MAX_HOTKEYS

typedef struct HotkeyDB HotkeyDB;
typedef struct HotkeyDBIterator HotkeyDBIterator;
typedef struct hotkey hotkeylist[MAX_HOTKEYS];


struct HotkeyDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(HotkeyDBIterator* self);

	/// Fetches the next hotkey list and stores it in 'data'.
	/// @param self Iterator
	/// @param data a character's hotkey list
	/// @param key a characters's char_id
	/// @return true if successful
	bool (*next)(HotkeyDBIterator* self, hotkeylist* data, int* key);
};


struct HotkeyDB
{
	bool (*init)(HotkeyDB* self);
	void (*destroy)(HotkeyDB* self);

	bool (*sync)(HotkeyDB* self);

	bool (*remove)(HotkeyDB* self, const int char_id);

	bool (*save)(HotkeyDB* self, const hotkeylist* list, const int char_id);
	bool (*load)(HotkeyDB* self, hotkeylist* list, const int char_id);

	/// Returns an iterator over all hotkey lists.
	///
	/// @param self Database
	/// @return Iterator
	HotkeyDBIterator* (*iterator)(HotkeyDB* self);
};


#endif /* _HOTKEYDB_H_ */
