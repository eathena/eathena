

#pragma once

class Lock {
	public:
		static FILE*	open(const char *filename, int *info);
		static int	close(FILE *fp, const char *filename, int *info);
};