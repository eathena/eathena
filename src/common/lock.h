#pragma once

class Lock {
	public:
		FILE*	open(const char *filename, int *info);
		int	close(FILE *fp, const char *filename, int *info);
};