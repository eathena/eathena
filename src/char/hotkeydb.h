// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOTKEYDB_H_
#define _HOTKEYDB_H_


#include "../common/mmo.h" // struct hotkey, MAX_HOTKEYS
#include "csdbiterator.h"


typedef struct HotkeyDB HotkeyDB;
typedef struct hotkey hotkeylist[MAX_HOTKEYS];


struct HotkeyDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(HotkeyDB* self);
		void (*destroy)(HotkeyDB* self);
		bool (*sync)(HotkeyDB* self, bool force);
	} p;

	bool (*remove)(HotkeyDB* self, const int char_id);

	bool (*save)(HotkeyDB* self, const hotkeylist* list, const int char_id);
	bool (*load)(HotkeyDB* self, hotkeylist* list, const int char_id);

	/// Returns an iterator over all hotkey lists.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(HotkeyDB* self);
};


#endif // _HOTKEYDB_H_
