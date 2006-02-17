#include "basesocket.h"



void test_socket()
{
#ifdef DEBUG
	{
		minisocket ms;
		ms.connect("http://checkip.dyndns.org");
		const char query[] = "GET / HTTP/1.1\r\nHost: checkip.dyndns.org\r\n\r\n";
		ms.write((const unsigned char*)query, strlen(query));
		if( ms.waitfor(1000) )
		{
			unsigned char buffer[1024];
			ms.read(buffer, sizeof(buffer));
			buffer[1023]=0;

			CRegExp regex("Current IP Address:\\s+([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)");
			regex.match((const char*)buffer);
			ipaddress myip = (const char*)regex[1];

			printf("ipaddress is: %s", (const char*)tostring(myip));
		}
	}
#endif//DEBUG
}



