// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_CORE_H_
#define	_CORE_H_

extern char *argp;
extern int runflag;
extern unsigned long ticks;
extern char SERVER_TYPE;

extern const char *get_svn_revision();
extern int do_init(int,char**);
extern void set_server_type(void);
extern void set_termfunc(void (*termfunc)(void));
extern void do_final();

#endif	// _CORE_H_
