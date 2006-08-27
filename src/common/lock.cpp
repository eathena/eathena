// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basereentrant.h"
#include "lock.h"
#include "utils.h"
#include "showmsg.h"

#ifndef WIN32

#include <unistd.h>

#else

#include <io.h>	// for access

#define F_OK   0x0
#define R_OK   0x4

/*
00 Existence only 
02 Write permission 
04 Read permission 
06 Read and write permission 
*/
#endif







#define exists(filename) (!access(filename, F_OK))


// 書き込みファイルの保護処理
// （書き込みが終わるまで、旧ファイルを保管しておく）

// 新しいファイルの書き込み開始
FILE* lock_fopen (const char* filename, int &info)
{
	char newfile[512];
	FILE *fp;
	int no = 0;

	// 安全なファイル名を得る（手抜き）
	do {
		snprintf(newfile, sizeof(newfile),"%s_%04d.tmp", filename, ++no);
	} while((fp = basics::safefopen(newfile,"r")) && (fclose(fp), no<9999) );
	info = no;
	return basics::safefopen(newfile,"wb");
}

// 旧ファイルを削除＆新ファイルをリネーム
int lock_fclose (FILE *fp, const char* filename, int info)
{
	int ret = 1;
	char newfile[512];
	char oldfile[512];
	if (fp != NULL)
	{
		ret = fclose(fp);
		snprintf(newfile, sizeof(newfile),"%s_%04d.tmp", filename, info);
		snprintf(oldfile, sizeof(oldfile),"%s.bak", filename);	// old backup file

		if (exists(oldfile)) remove(oldfile);	// remove backup file if it already exists
		rename (filename, oldfile);				// backup our older data instead of deleting it

		// このタイミングで落ちると最悪。
		if ((ret = rename(newfile,filename)) != 0)
		{	// rename our temporary file to its correct name
			char errstr[256];
			ShowError("%s - '"CL_WHITE"%s"CL_RESET"'\n", strerror(errno, errstr, 256), newfile);
		}
	}
	return ret;
}

