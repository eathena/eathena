#include "lock.h"	// Lock Header
#include <errno.h>

Lock::open(const char *filename,int *info)
{
	char newfile[512];
	FILE *fp;
	int no=0;

	do
	{
		sprintf(newfile, "%s_%04d.tmp", filename, ++no);
	} while((fp = fopen(newfile,"r")) && (fclose(fp), no<9999));

	*info = no;

	return fopen(newfile, "w");
}

Lock::close(FILE *fp, const char *filename,int *info)
{
	char newfile[512];
	int ret=0;
	if(!fp)
	{
		return -1;
	} else {
		ret = fclose(fp);
		sprintf(newfile, "%s_%04d.tmp", filename, *info);
		remove(filename);
		if(rename(newfile, filename) != 0)
		{
			sprintf(tmp_output,"%s - '"CL_WHITE"%s"CL_RESET"'\n", strerror(errno), newfile);
			ShowError(tmp_output);
		}
		return ret;
	}
}