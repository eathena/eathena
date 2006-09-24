#include "basearray.h"
#include "baseinet.h"
#include "basebuffer.h"
#include "basesync.h"
#include "basethreads.h"
#include "basesocket.h"
#include "basestring.h"
#include "baseregex.h"


NAMESPACE_BEGIN(basics)



///////////////////////////////////////////////////////////////////////////////
#ifndef WIN32
///////////////////////////////////////////////////////////////////////////////
// dynamic size, unix system independend fd_set replacement

///////////////////////////////////////////////////////////////////////////
// resize the array; only grow, no shrink
void CFDSET::checksize(size_t pos)
{	// pos gives the dword position in the array
	if( pos >= cSZ )
	{	// need to reallocate
		size_t sz = (this->cSZ)?this->cSZ:2;
		while(sz <= pos) sz *= 2;

		unsigned long* temp= new unsigned long[sz];

		// copy over the old array
		if(cArray)
		{
			memcpy(temp, cArray, this->cSZ*sizeof(unsigned long));
			delete[] cArray;
		}
		// and clear the rest
		memset(temp+this->cSZ,0,(sz-this->cSZ)*sizeof(unsigned long));

		// take it over
		cArray = temp;
		cSZ = sz;
	}
}
void CFDSET::copy(const CFDSET& cfd)
{
	if(this != &cfd)
	{
		if( cfd.cSZ > this->cSZ )
		{	// not enough space, need to realloc
			if(cArray) delete [] cArray;
			this->cSZ = cfd.cSZ;
			cArray = new unsigned long[this->cSZ];
		}
		else
		{	// current array is larger, just clear the uncopied range
			memset(cArray+cfd.cSZ,0, (this->cSZ-cfd.cSZ)*sizeof(unsigned long));
		}
		// and copy the given array if it exists
		if(cfd.cArray)
			memcpy(cArray, cfd.cArray, cfd.cSZ*sizeof(unsigned long));
	}
}

///////////////////////////////////////////////////////////////////////////
// Call a function with each set bit
// version 1 (using log2)
size_t CFDSET::foreach1( void(*func)(SOCKET), size_t max) const
{
	size_t c = 0;
	if(func)
	{
		SOCKET sock;
		unsigned long	val;
		unsigned long	bits;
		unsigned long	nfd=0;
		max = howmany(max, NFDBITS);
		if(max>this->cSZ) max=this->cSZ;

		while( nfd < max )
		{	// while something is set in the ulong at position nfd
			bits = cArray[nfd];
			while( bits )
			{	// method 1
				// calc the highest bit with log2 and clear it from the field
				// this method is especially fast 
				// when only a few bits are set in the field
				// which usually happens on read events
				val = log2( bits );
				bits ^= (1ul<<val);	
				// build the socket number
				sock = nfd*NFDBITS + val;

				///////////////////////////////////////////////////
				// call the user function
				func(sock);
				c++;
			}
			// go to next field position
			nfd++;
		}
	}
	return c;
}
///////////////////////////////////////////////////////////////////////////
// Call a function with each set bit
// version 2 (using shifts)
size_t CFDSET::foreach2( void(*func)(SOCKET), size_t max ) const
{
	size_t c=0;
	if(func)
	{
		SOCKET sock;
		unsigned long	val;
		unsigned long	bits;
		unsigned long	nfd=0;
		max = howmany(max, NFDBITS);
		if(max>this->cSZ) max=this->cSZ;

		while( nfd <  max )
		{	// while something is set in the ulong at position nfd
			bits = cArray[nfd];
			val = 0;
			while( bits )
			{	// method 2
				// calc the next set bit with shift/add
				// therefore copy the value from fds_bits 
				// array to an unsigned type (fd_bits is an field of long)
				// otherwise it would not shift the MSB
				// the shift add method is faster if many bits are set in the field
				// which is usually valid for write operations on large fields
				while( !(bits & 1) )
				{
					bits >>= 1;
					val ++;
				}
				//calculate the socket number
				sock = nfd*NFDBITS + val;
				// shift one more for the next loop entrance
				bits >>= 1;
				val ++;

				///////////////////////////////////////////////////
				// call the user function
				func(sock);
				c++;
			}
			// go to next field position
			nfd++;
		}
	}
	return c; // number of processed sockets
}


#else 
///////////////////////////////////////////////////////////////////////////////
// dynamic size, windows system independend fd_set replacement

///////////////////////////////////////////////////////////////////////////
// resize the array; only grow, no shrink
void CFDSET::checksize()
{	// no pos parameter here
	if( cSet->fd_count >= this->cSZ )
	{	// need to reallocate
		size_t sz = (this->cSZ)?this->cSZ:2;
		while(sz <= cSet->fd_count) sz *= 2;

		struct winfdset *temp= (struct winfdset *)new char[sizeof(struct winfdset)+sz*sizeof(SOCKET)];

		// copy over the old array
		if(this->cSet)
		{
			memcpy(temp, this->cSet, sizeof(struct winfdset)+this->cSZ*sizeof(SOCKET));
			delete[] ((char*)(this->cSet));
		}
		// clearing the rest is not necessary

		// take it over
		this->cSet = temp;
		this->cSZ = sz;
	}
}
void CFDSET::copy(const CFDSET& cfd)
{
	if(this != &cfd)
	{
		if( cfd.cSet->fd_count > this->cSZ )
		{	// not enough space, need to realloc
			if(cSet) delete [] ((char*)cSet);
			
			this->cSZ = cfd.cSZ;
			this->cSet = (struct winfdset *) new char[sizeof(struct winfdset)+this->cSZ*sizeof(SOCKET)];
		}
		//else
		// current array is larger, nothing to do in this case

		// and copy the given array if it exists
		if(cfd.cSet)
			memcpy(this->cSet, cfd.cSet, sizeof(struct winfdset)+this->cSZ*sizeof(SOCKET));
	}
}
bool CFDSET::find(SOCKET sock, size_t &pos) const
{
	return BinarySearch<SOCKET,SOCKET*>(sock, this->cSet->fd_array, this->cSet->fd_count, 0, pos);
}
///////////////////////////////////////////////////////////////////////////
// set a bit
void CFDSET::set_bit(int fd)
{
	if(fd>0)
	{
		size_t pos;
		if( !this->find(fd, pos) )
		{
			this->checksize();
			memmove(this->cSet->fd_array+pos+1, this->cSet->fd_array+pos, (this->cSet->fd_count-pos)*sizeof(cSet->fd_array[0]));
			this->cSet->fd_array[pos] = fd;
			this->cSet->fd_count++;
		}	
	}		
}
///////////////////////////////////////////////////////////////////////////
// Clear a bit
void CFDSET::clear_bit(int fd)
{
	if(fd>0)
	{	
		size_t pos;
		if( this->find(fd, pos) )
		{
			memmove(this->cSet->fd_array+pos, this->cSet->fd_array+pos+1, (this->cSet->fd_count-pos-1)*sizeof(cSet->fd_array[0]));
			this->cSet->fd_count--;
		}
	}
}
///////////////////////////////////////////////////////////////////////////
// Clear a bit
bool CFDSET::is_set(int fd) const
{
	if(fd>0)
	{	
		size_t pos;
		return this->find(fd, pos);
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////
// Call a function with each set bit
size_t CFDSET::foreach1( void(*func)(SOCKET), size_t max) const
{
	if(func)
	{	
		size_t i;
		for(i=0; i<this->cSet->fd_count; ++i)
			func( this->cSet->fd_array[i] );
	}
	return cSet->fd_count;
}
#endif











///////////////////////////////////////////////////////////////////////////////
// dynamic id's
///////////////////////////////////////////////////////////////////////////////

void CIDClient::receive(identifier id)
{	// id is the starting point from where we generate IDSERVER_AMOUNT id's

	if(id)
	{
		const identifier e = id+IDSERVER_AMOUNT;

		CIDVal* cid;
		// put id's to existing ID's without values
		for( ; id<e && idrefs.size(); ++id)
		{
			cid = (CIDVal*)idrefs.pop();
			if(cid)
				cid->id = id;
			else
				idlist.push(id);
		}
		// insert the rest, values shuffled
		for( ; id<e; ++id)
		{
			const size_t i=idlist.size();
			idlist.insert(id, 1, (i|(i-1))&rand());
		}

		// request more id's when running empty
		requesting = ( idlist.size() <= IDSERVER_AMOUNT/8 ) ? (*this->sendreqest)() : false;
	}
}

identifier CIDClient::aquire()
{
	// request more id's when running empty
	if( !requesting && idlist.size() < 8 )
		requesting = (*this->sendreqest)();

	if( idlist.size() )
		 return idlist.pop();
	return 0;
}
void CIDClient::release(identifier id)
{
	if(id) idlist.push(id);
}
void CIDClient::aquire(CIDVal& id)
{
	// request more id's when running empty
	if( !requesting && idlist.size() < IDSERVER_AMOUNT/8 )
		requesting = (*this->sendreqest)();

	if( idlist.size() )
		id.id = idlist.pop();
	else
	{
		id.id = 0;
		// store the pointer so it can be updated later
		idrefs.push(&id);
	}
}
void CIDClient::release(CIDVal& id)
{
	if(id.id)
		idlist.push(id.id);
	else
	{	// deleting some zero id
		// find it in list
		size_t pos;
		if( idrefs.find(&id,0,pos) )
			idrefs.removeindex(pos);
	}
}

#ifdef DEBUG
CIDServer idserv;

bool isrequest();
CIDClient CIDVal::idclient(isrequest);


bool isrequest()
{

	// client code sends request to server->
	// returns true when sending is on it's way, false on failure

	// -> server code call idserver request
	uint64 id = idserv.request();
	// server code sends id to client ->
	// no further error passing

	// -> client code calls idclient receive
	CIDVal::idclient.receive(id);
	// no error passing

	// this example returns false since the receive has been done already
	return false;
}
#endif//DEBUG


void test_id()
{
#ifdef DEBUG
	CIDVal a,b,c,d;

	uint64 i = CIDVal::idclient.aquire();

	for(i=0; i<CIDVal::idclient.idlist.size(); ++i)
		printf("%i ", CIDVal::idclient.idlist[(size_t)i]); 
#endif//DEBUG
}




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
			ipaddress myip = ipaddress(regex[1]);

			printf("ipaddress is: %s", (const char*)tostring(myip));
		}
	}

	{
		ipaddress sysip = ipaddress::GetSystemIP();
		ipaddress ip = hostbyname("checkip.dyndns.org");
		//ipaddress ip = hostbyname("www.google.com");

		struct sockaddr_in addr;
		SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );

		addr.sin_family			= AF_INET;
		addr.sin_addr.s_addr	= htonl( ip );
		addr.sin_port			= htons(80);
		
		if( 0 > connect(sock, (struct sockaddr *)(&addr),sizeof(struct sockaddr_in)) )
		{
			closesocket(sock);
		}
		else
		{
			const char* str = 
			 "GET / HTTP/1.1\r\n"
			 "Host: checkip.dyndns.org\r\n"
//			 "User-Agent: Mozilla/5.0 (compatible; MSIE 5.0; IRIX 6.3 IP32)\r\n"
//			 "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
//			 "Accept-Language: de-de,de;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
//			 "Accept-Encoding: gzip,deflate\r\n"
//			 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
			 "Keep-Alive: 300\r\n"
			 "Connection: keep-alive\r\n"
			"\r\n";
			int ret = write(sock, (char*)str, strlen(str));
			if(ret != (int)strlen(str))
				printf("send err");

			unsigned long arg = 0;
			char buffer[1024];

			fd_set fdset;
			FD_ZERO(&fdset);
			FD_SET(sock,&fdset);

			select(sock+1,&fdset,NULL,NULL,NULL);

			int rv = ioctlsocket(sock, FIONREAD, &arg);
			if( (rv == 0) && (arg > 0) )
			{	// we are reading 'arg' bytes of data from the socket
				
				if( arg > sizeof(buffer) ) arg = sizeof(buffer);
				int len=read(sock,buffer,arg);
				buffer[len]=0;

				FILE*ff = fopen("test2.txt", "wb");
				fwrite(buffer,1,len,ff);
				fclose(ff);

				CRegExp regex("Current IP Address:\\s+([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)");
				regex.match(buffer);

				ipaddress myip = ipaddress(regex[1]);
			}
		}
	}


	{
		unsigned long ip = ipaddress::GetSystemIP(); 
		unsigned short port = 8080;

		struct sockaddr_in server_address;
		SOCKET server;
		int result;

		server = socket( AF_INET, SOCK_STREAM, 0 );

		server_address.sin_family      = AF_INET;
		server_address.sin_addr.s_addr = htonl( ip );
		server_address.sin_port        = htons(port);

		result = bind(server, (struct sockaddr*)&server_address, sizeof(server_address));
		if( result == -1 ) {
			closesocket(server);
			perror("bind");
			exit(1);
		}
		result = listen( server, 5 );
		if( result == -1 ) {
			closesocket(server);
			perror("listen");
			exit(1);
		}

		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(server,&fdset);

		select(server+1,&fdset,NULL,NULL,NULL);


		
		SOCKET client;
		struct sockaddr_in client_address;
		socklen_t len = sizeof(client_address);

		client = accept(server,(struct sockaddr*)&client_address,&len);
		if(client==-1) 
		{	// same here, app might have passed away
			perror("accept");
			return;
		}


		unsigned long arg = 0;
		int rv = ioctlsocket(client, FIONREAD, &arg);
		if( (rv == 0) && (arg > 0) )
		{	// we are reading 'arg' bytes of data from the socket
			char buffer[1024];
			if( arg > sizeof(buffer) ) arg = sizeof(buffer);
			int len=read(client,buffer,arg);
			buffer[len]=0;
			FILE*ff = fopen("test.txt", "wb");
			fprintf(ff, buffer);
			fclose(ff);

			printf(buffer);
		}
	}



	{
		size_t i;
#ifdef WIN32
		ipaddress ip = ipaddress::GetSystemIP();
		struct winfdset
		{
			u_int fd_count;					// how many are SET?
			SOCKET  fd_array[2*FD_SETSIZE];	// an array of SOCKETs 
											// only one in the struct the others will be alloced outside
		} fd;
		for(i=0; i<2*FD_SETSIZE; ++i)
			fd.fd_array[i] = i;
		fd.fd_count = 0;
#else
		struct uxfdset
		{
			ulong fd_array[ howmany(2*FD_SETSIZE,NBBY*sizeof(ulong)) ];
		} fd;
		memset (fd.fd_array, 0xFF, sizeof(fd.fd_array));
		FD_ZERO((fd_set *)&fd);
		fd.fd_array[1] = ~0;


#endif
		SOCKET s1 = ::socket( AF_INET, SOCK_STREAM, 0 );
		SOCKET s2 = ::socket( AF_INET, SOCK_STREAM, 0 );
		SOCKET s3 = ::socket( AF_INET, SOCK_STREAM, 0 );
		SOCKET s4 = ::socket( AF_INET, SOCK_STREAM, 0 );

#ifndef WIN32
		if(s1>2*FD_SETSIZE)
		{
			printf("s1 too large");
		}
		else
#endif
		{
			if(s1>FD_SETSIZE)
				printf("s1 outside");
			FD_SET(s1, (fd_set *)&fd);
		}
		printf("  %i\n", (int)s1);
#ifndef WIN32
		if(s2>2*FD_SETSIZE)
		{
			printf("s2 too large");
		}
		else
#endif
		{
			if(s2>FD_SETSIZE)
				printf("s2 outside");
			FD_SET(s2, (fd_set *)&fd);
		}
		printf("  %i\n", (int)s2);
#ifndef WIN32
		if(s3>2*FD_SETSIZE)
		{
			printf("s3 too large");
		}
		else
#endif
		{
			if(s3>FD_SETSIZE)
				printf("s3 outside");
			FD_SET(s3, (fd_set *)&fd);
		}
		printf("  %i\n", (int)s3);
#ifndef WIN32
		if(s4>2*FD_SETSIZE)
		{
			printf("s4 too large");
		}
		else
#endif
		{
			if(s4>FD_SETSIZE)
				printf("s4 outside");
			FD_SET(s4, (fd_set *)&fd);
		}
		printf("  %i\n", (int)s4);

		static struct timeval timeout  = {0,0};
		


		struct sockaddr_in server_address;
		server_address.sin_family      = AF_INET;
		server_address.sin_addr.s_addr = htonl( ipaddress::GetSystemIP() );
		server_address.sin_port        = htons( 1414 );
		int result = bind(s1, (struct sockaddr*)&server_address, sizeof(server_address));
		if( result == -1 ) {
			exit(1);
		}
		result = listen( s1, 5 );
		if( result == -1 ) {
			exit(1);
		}

		result = connect(s2, (struct sockaddr *)(&server_address),sizeof(struct sockaddr_in));


		int num = ::select(FD_SETSIZE/2,NULL,(fd_set *)&fd,NULL,&timeout);

		printf("%i\n", num);
		for(i=0; i<howmany(2*FD_SETSIZE,NBBY*sizeof(ulong)); ++i)
			printf("%X ", fd.fd_array[i]);
		printf("\n");

		num = ::select(FD_SETSIZE,(fd_set *)&fd,NULL,NULL,&timeout);

		printf("%i\n", num);
		for(i=0; i<howmany(2*FD_SETSIZE,NBBY*sizeof(ulong)); ++i)
			printf("%X ", fd.fd_array[i]);
		printf("\n");
		

	}


#endif//DEBUG
}











#ifdef DEBUG
#include "basearray.h"
#include "baseinet.h"
#include "basesync.h"
#include "basesocket.h"

// complete client server testcase


class socketprocessor : public noncopyable
{
public:
	SOCKET sock;
	socketprocessor(SOCKET s=INVALID_SOCKET) : sock(s)	{}
	virtual ~socketprocessor()								{}

	virtual int run()=0;
};

class testproc;
class testserver : public socketprocessor
{
	friend class testproc;
public:
	CFDSET fds;
	static smap<SOCKET, socketprocessor*> sessionmap;
	static void processor(SOCKET s)
	{
		if( sessionmap.exists(s) )
		{
			//printf("process %i\n", s);
			sessionmap[s]->run();
			//printf("done %i\n", s);
		}
	}
	static void processorprint(SOCKET s)
	{
		printf("%i ", s);
	}

	testserver(const unsigned short port)
	{
		this->sock = socket( AF_INET, SOCK_STREAM, 0 );
		if( this->sock!= INVALID_SOCKET )
		{
			struct sockaddr_in server_address;
			server_address.sin_family      = AF_INET;
			server_address.sin_addr.s_addr = htonl( ipany );
			server_address.sin_port        = htons( port );

			if( -1 != bind(this->sock, (struct sockaddr*)&server_address, sizeof(server_address)) &&
				-1 != ::listen(this->sock, SOMAXCONN) )
			{
				this->fds.set_bit(this->sock);
				this->sessionmap[this->sock] = this;
				printf("listening at port %i\n", port);
			}
			else
			{
				closesocket(this->sock);
				perror("bind");
			}
		}
	}

	virtual ~testserver()	{}

	virtual int run();



	void listen()
	{

		for(;;)
		{
			CFDSET tmp = this->fds;
			struct timeval timeout = {1,0};
			int i;
			
			//printf("start select %i with ", (int)tmp.size());
			//tmp.foreach1(&this->processorprint, this->fds.size());
			//printf("\n");
			
			if( (i=::select(tmp.size(), tmp, NULL, NULL, &timeout))>0 )
			{
				//printf("select %i: ",i);
				//tmp.foreach1(&this->processorprint, this->fds.size());
				//printf("\n");
				tmp.foreach1(&this->processor, this->fds.size());
			}
			else
			{
				printf("[%5i]\r", (int)this->sessionmap.size());fflush(stdout);
			}
		}
	}
};
smap<SOCKET, socketprocessor*> testserver::sessionmap;

class testproc : public socketprocessor
{
	testserver& server;
public:

	testproc(SOCKET s, testserver& t) : socketprocessor(s), server(t)
	{
		server.sessionmap[this->sock] = this;
		server.fds.set_bit(this->sock);

	}
	virtual ~testproc()
	{
		if(this->sock!=INVALID_SOCKET)
		{
			server.sessionmap.erase(this->sock);
			server.fds.clear_bit(this->sock);
			closesocket(this->sock);
		}
	}

	virtual int run()
	{
		//printf("run testproc %p (socket %i) ", this, this->sock); fflush(stdout);
		unsigned long arg=0, len;
		int rv = ioctlsocket(this->sock, FIONREAD, &arg);
		//printf("(ioctlsocket %i len=%lu) ", rv, arg); fflush(stdout);
		
		char* buffer=NULL;
		if( (rv == 0) && (arg > 0) && 
			(buffer=new char[1+arg]) &&
			(len=read(this->sock,buffer,arg)) )
		{
			buffer[len]=0;
			printf("[%5i]recv %p,%i (len=%lu): '%s' \n", (int)server.sessionmap.size(), this, this->sock, len, buffer); fflush(stdout);
		}
		else
		{
			printf("terminating\n"); fflush(stdout);
			delete this;
		}
		if(buffer) delete[] buffer;
		return arg;
	}
};

int testserver::run()
{
	struct sockaddr_in client_address;
	socklen_t len = sizeof(client_address);
	SOCKET s = accept(this->sock,(struct sockaddr*)&client_address,&len);
	//printf("accept %i\n", s);
	
	if(s!=INVALID_SOCKET) 
	{
		testproc* p = new testproc(s, *this);
		printf("incoming from %X (%p = #%i)(socket %i)\n", ntohl(client_address.sin_addr.s_addr), p, (int)this->sessionmap.size(),s );
	}
	else
	{
		printf("error: %s\n", sockerrmsg(sockerrno()) );
	}
	return 0;
}

class testclient : public socketprocessor
{
public:
	netaddress ipaddr;
	testclient(const netaddress& ip) : ipaddr(ip)
	{
		this->reconnect();
	}
	virtual ~testclient()
	{
		if( this->sock!=INVALID_SOCKET )
			closesocket(this->sock);
	}
	virtual int run()
	{
		//printf("run testproc %p (socket %i) ", this, this->sock); fflush(stdout);
		unsigned long arg=0, len;
		int rv = ioctlsocket(this->sock, FIONREAD, &arg);
		//printf("(ioctlsocket %i len=%lu) ", rv, arg); fflush(stdout);
		
		char* buffer=NULL;
		if( (rv == 0) && (arg > 0) && 
			(buffer=new char[1+arg]) &&
			(len=read(this->sock,buffer,arg)) )
		{
			buffer[len]=0;
			printf("[%5i]recv %p,%i (len=%lu): '%s' \n", 0, this, this->sock, len, buffer); fflush(stdout);
		}
		else
		{
			printf("terminating\n"); fflush(stdout);
			this->sock=INVALID_SOCKET;
		}
		if(buffer) delete[] buffer;
		return arg;
	}

	bool reconnect()
	{
		this->sock = socket( AF_INET, SOCK_STREAM, 0 );
		if(this->sock!=INVALID_SOCKET) 
		{
			struct sockaddr_in addr;
			addr.sin_family			= AF_INET;
			addr.sin_addr.s_addr	= htonl( ipaddr.addr() );
			addr.sin_port			= htons( ipaddr.port() );
			if( 0 > connect(this->sock, (struct sockaddr *)(&addr),sizeof(struct sockaddr_in)) )
			{
				closesocket(this->sock);
				this->sock = INVALID_SOCKET;
				printf("connect failed\n");
			}
			else
				return true;
		}
		return false;
	}
	int send(const char* str)
	{
		return ( str && (this->sock!=INVALID_SOCKET || this->reconnect()) ) ? write(this->sock, (char*)str, strlen(str)) : 0;
	}
};


int testsocket(int argc, char *argv[])
{

	if(argc>2)
	{
		if( 0==strcasecmp(argv[1],"server") )
		{	// start with "server <port>"
			unsigned short port = atoi(argv[2]);
			testserver serv(port);
			serv.listen();

		}
		else if( 0==strcasecmp(argv[1],"client") && argc>3 )
		{	// start with "client <number of connections> <address:port>"
			size_t i,count= atoi(argv[2]);
			netaddress ip = argv[3];

			vector<testclient*> clients;
			for(i=0; i<count; ++i)
			{
				clients.push( new testclient(ip) );
			}

			for(;;)
			{
				for(i=0; i<clients.size(); ++i)
					clients[i]->send("...test..."), sleep(100);
				sleep(500);
			}

		}

	}


	return 0;
}

#endif//DEBUG

NAMESPACE_END(basics)
