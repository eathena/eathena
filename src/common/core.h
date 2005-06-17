// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_CORE_H_
#define	_CORE_H_

//#define SQL_DEBUG //uncomment for debug_mysql_query instead of mysql_real_query

//Added here, so its avail in 'all' files ..
#define eprintf(mes, args...) \
        fprintf(stderr, "%s:%d: "mes"", __FILE__, __LINE__, args);
#define eprint(mes) \
	fprintf(stderr, "%s:%d: "mes"", __FILE__, __LINE__);

extern int arg_c;
extern char **arg_v;

extern int runflag;
extern char *SERVER_NAME;
extern char SERVER_TYPE;

extern const char *get_svn_revision();
extern int do_init(int,char**);
extern void set_server_type(void);
extern void set_termfunc(void (*termfunc)(void));
extern void do_final();

#endif	// _CORE_H_
