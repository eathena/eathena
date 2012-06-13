#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include "mysql.h"
int main(int argc, char** argv)
{
	MYSQL* h = mysql_init(0);
	if(h == 0) return 1;//error
	mysql_close(h);
	return 0;
}
