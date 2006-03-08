#include "basearray.h"
#include "baseinet.h"
#include "basebuffer.h"
#include "basesync.h"
#include "basethreads.h"
#include "basesocket.h"
#include "basestring.h"
#include "baseregex.h"





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
		while(sz >= pos) sz *= 2;

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
size_t CFDSET::foreach1( void(*func)(size_t), size_t max) const
{
	size_t c = 0;
	if(func)
	{
		size_t fd;
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
				bits ^= (1<<val);	
				// build the socket number
				sock = nfd*NFDBITS + val;

				///////////////////////////////////////////////////
				// call the user function
				func(fd);
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
size_t CFDSET::foreach2( void(*func)(size_t), size_t max ) const
{
	size_t c=0;
	if(func)
	{
		size_t fd;
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
				func(fd);
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
		while(sz >= cSet->fd_count) sz *= 2;

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
size_t CFDSET::foreach1( void(*func)(size_t), size_t max) const
{
	if(func)
	{	
		size_t i;
		for(i=0; i<this->cSet->fd_count; i++)
			func( this->cSet->fd_array[i] );
	}
	return cSet->fd_count;
}
#endif









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



