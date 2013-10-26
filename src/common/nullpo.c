// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nullpo.h"
#include "showmsg.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static void nullpo_info_core(const char *file, int line, const char *func);

/*======================================
 * Checks for and reports NULL pointers
 *--------------------------------------*/
int nullpo_chk(const char *file, int line, const char *func, const void *target)
{
	if (target != NULL)
		return 0;

	nullpo_info_core(file, line, func);
	return 1;
}

/*======================================
 * nullpo information output (Main)
 *--------------------------------------*/
static void nullpo_info_core(const char *file, int line, const char *func)
{
	if (file == NULL)
		file = "??";

	func =
		func == NULL    ? "unknown":
		func[0] == '\0' ? "unknown":
		                  func;

	ShowMessage("--- nullpo info --------------------------------------------\n");
	ShowMessage("%s:%d: in func `%s'\n", file, line, func);
	ShowMessage("--- end nullpo info ----------------------------------------\n");

	// ここらでnullpoログをファイルに書き出せたら
	// まとめて提出できるなと思っていたり。
}
