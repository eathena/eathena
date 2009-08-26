#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charlogdb.h"
#include <stdarg.h>
#include <string.h>


// CharLogDB storage engines available
static struct {
	CharLogDB* (*constructor)(void);
	CharLogDB* engine;
} charlog_engines[] = {
// standard engines
#ifdef WITH_SQL
	{charlog_db_sql, NULL},
#endif
#ifdef WITH_TXT
	{charlog_db_txt, NULL},
#endif
};


bool charlog_enabled = true;
char charlog_engine[256] = "auto"; // name of the engine to use (defaults to auto, for the first valid engine)
CharLogDB* charlog = NULL; // charlog object


void charlog_create(void)
{
	int i;

	// create charlog engines (to accept config settings)
	for( i = 0; i < ARRAYLENGTH(charlog_engines); ++i )
		if( charlog_engines[i].constructor )
			charlog_engines[i].engine = charlog_engines[i].constructor();
}


void charlog_config_read(const char* w1, const char* w2)
{
	if( strcmpi(w1, "charlog.enabled") == 0 )
		charlog_enabled = true;
	else
	if( strcmpi(w1, "charlog.engine") == 0 )
		safestrncpy(charlog_engine, w2, sizeof(charlog_engine));
	else
	{// try charlog engines
		int i;
		for( i = 0; i < ARRAYLENGTH(charlog_engines); ++i )
		{
			CharLogDB* engine = charlog_engines[i].engine;
			if( engine )
				engine->set_property(engine, w1, w2);
		}
	}
}


bool charlog_init(void)
{
	int i;
	bool try_all = (strcmp(charlog_engine,"auto") == 0);

	if( !charlog_enabled )
		return true;

	for( i = 0; i < ARRAYLENGTH(charlog_engines); ++i )
	{
		char name[sizeof(charlog_engine)];
		CharLogDB* engine = charlog_engines[i].engine;
		if( engine && engine->get_property(engine, "engine.name", name, sizeof(name)) &&
			(try_all || strcmp(name, charlog_engine) == 0) )
		{
			if( !engine->init(engine) )
			{
				ShowError("charlog_init: failed to initialize engine '%s'.\n", name);
				continue;// try next
			}
			if( try_all )
				safestrncpy(charlog_engine, name, sizeof(charlog_engine));
			ShowInfo("Using charlog engine '%s'.\n", charlog_engine);
			charlog = engine;
			return true;
		}
	}
	ShowError("charlog_init: charlog engine '%s' not found.\n", charlog_engine);
	return false;
}


void charlog_final(void)
{
	int i;
	for( i = 0; i < ARRAYLENGTH(charlog_engines); ++i )
	{// destroy all charlog engines
		CharLogDB* engine = charlog_engines[i].engine;
		if( engine )
		{
			engine->destroy(engine);
			charlog_engines[i].engine = NULL;
		}
	}
	charlog = NULL;
}


void charlog_log(int char_id, int account_id, int slot, const char* name, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);

	if( !charlog_enabled || charlog == NULL )
		return;

	charlog->log(charlog, char_id, account_id, slot, name, msg, ap);

	va_end(ap);
}
