#pragma once
#ifndef SIGPIPE
	#define SIGPIPE SIGINT
#endif
#if defined(WIN32)
		#define WIN32_LEAN_AND_MEAN
#endif

class Core {
	public:
		// Constructors
		Core(void);
		Core(int argc, char **argv);
		void		SetTermFunc(void (*termfunc)(void));

		static void	Sig_Proc(int sn);
		static int	Get_SVNRev(char *svnentry);
		static void	Display_Title(void);
		sigfunc*	compat_signal(int signo, sigfunc *func);
	private:
		int runflag;
};