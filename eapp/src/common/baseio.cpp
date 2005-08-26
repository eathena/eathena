#include "base.h"
#include "baseio.h"


/////////////////////////////////////////////////////////////////////
// a text based database with extra index
// not much tested though and quite basic
// but should be as fast as file access allows 
// faster than sql anyway + threadsafe (not shareable though)
/////////////////////////////////////////////////////////////////////
#define DB_OPCOUNTMAX 100	// # of db ops before forced cache flush

class txt_database : private Mutex
{
	/////////////////////////////////////////////////////////////////
	// index structure
	class _key
	{
	public:
		ulong cKey;	// key value
		ulong cPos;	// position in file
		ulong cLen;	// data length

		_key(ulong k=0, ulong p=0, ulong l=0):cKey(k), cPos(p), cLen(l)	{}
		bool operator==(const _key& k) const	{ return cKey==k.cKey; }
		bool operator> (const _key& k) const	{ return cKey> k.cKey; }
		bool operator< (const _key& k) const	{ return cKey< k.cKey; }
	};
	/////////////////////////////////////////////////////////////////
	// class data
	char *cName;			// path/name of db file
	FILE *cDB;				// file handle to db
	FILE *cIX;				// file handle to index

	size_t cOpCount;		// count of operations

	TslistDST<_key>	cIndex;	// the index

	/////////////////////////////////////////////////////////////////
public:
	/////////////////////////////////////////////////////////////////
	// construction/destruction
	txt_database() : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{}
	~txt_database()
	{
		close();
	}

	/////////////////////////////////////////////////////////////////
	// open the database, create when not exist
	// extension is always ".txt" for db and ".inx" for index
	bool open(const char *name)
	{
		ScopeLock sl(*this);
		char *ip;
		
		if(NULL==name)
			return false;

		close();

		// with some extra space for file extension
		cName = new char[5+strlen(name)]; 
		// copy and correct the path seperators
		checkpath(cName, name);

		ip = strrchr(cName, '.');		// find a point in the name
		if(!ip || strchr(ip, PATHSEP) )	// if name had no point					
			ip = cName+strlen(cName);	// take all as name

		strcpy(ip, ".txt");
		cDB = fopen(cName, "rb+");
		if(!cDB)
		{	// either not exist or locked
			cDB = fopen(cName, "wb+");
			if(!cDB)	// locked
				return false;
		}

		strcpy(ip, ".inx");
		cIX = fopen(cName, "rb+");
		if(!cIX)
		{	// either not exist or locked
			cIX = fopen(cName, "wb+");
			if(!cIX)	// locked
			{
				fclose(cIX);
				return false;
			}
		}
		// cut the file name back to default
		*ip=0;

		// read index
		// structure is:
		// <# of entries> \n <i>(0), <p>(0), <l>(0) \n ...
		fseek(cIX, 0, SEEK_SET);
		unsigned long sz, p, l;
		if( 1==fscanf(cIX,"%li\n", &sz) )
		{
			cIndex.realloc(sz);
			while( 3==fscanf(cIX,"%li,%li,%li\n",  &sz, &p, &l) )
			{
				cIndex.insert( _key(sz,p,l) );
			}
		}

		// rebuild the database at startup might be not that expensive
		rebuild();

		return true;
	}
	/////////////////////////////////////////////////////////////////
	// closes the database
	bool close()
	{
		ScopeLock sl(*this);
		flush();
		// rebuild at close either, but might need to test
		rebuild();

		if(cDB) 
		{
			fclose(cDB);
			cDB = NULL;
		}
		if(cIX) 
		{
			fclose(cIX);
			cIX = NULL;
		}
		if(cName)
		{
			delete[] cName;
			cName = NULL;
		}
		return true;
	}
	/////////////////////////////////////////////////////////////////
	// flushes cache to files
	bool flush(bool force=false)
	{	
		ScopeLock sl(*this);
		bool ret = true;
		// check if necessary
		cOpCount++;
		if( force || cOpCount > DB_OPCOUNTMAX )
		{	// need to flush
			if(cDB) 
			{
				// nothing to flush here right now
			}
			if(cIX) 
			{
				// write index
				fseek(cIX, 0, SEEK_SET);
				fprintf(cIX,"%li\n", cIndex.size());
				for(size_t i=0; i<cIndex.size(); i++)
				{
					fprintf(cIX,"%li,%li,%li\n", 
						cIndex[i].cKey, cIndex[i].cPos, cIndex[i].cLen);
				}
				fflush(cIX);
			}
			// reset op counter
			cOpCount=0;
		}
		return ret;
	}
	/////////////////////////////////////////////////////////////////
	// insert/udate
	bool insert(const ulong key, char* data)
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		ulong len = strlen(data);
		size_t i;
		if( cIndex.find( _key(key), 0, i) )
		{	// update
			if( cIndex[i].cLen >= len )
			{	// rewrite the old position
				fseek(cDB, cIndex[i].cPos, SEEK_SET);
			}
			else
			{	// insert new at the end
				fseek(cDB, 0, SEEK_END);
			}
			cIndex[i].cLen = len;
			cIndex[i].cPos = ftell(cDB);
		}
		else
		{	// insert new at the end
			fseek(cDB, 0, SEEK_END);
			cIndex.insert( _key(key, ftell(cDB), len) );
		}
		fwrite(data, 1, len, cDB);

		this->flush();
		return true;
	}
	/////////////////////////////////////////////////////////////////
	// delete
	bool remove(const ulong key)
	{
		ScopeLock sl(*this);
		size_t pos;
		if( cIndex.find( _key(key), 0, pos) )
		{
			cIndex.removeindex(pos);
			this->flush();
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// search
	bool find(const ulong key, char* data) const
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		size_t i;
		if( cIndex.find( _key(key), 0, i) )
		{
			fseek(cDB, cIndex[i].cPos, SEEK_SET);
			fread(data, cIndex[i].cLen, 1, cDB);
			data[cIndex[i].cLen]=0;
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// rebuild database and index
	bool rebuild()
	{
		if(!cName || !cDB || !cIX)
			return false;

		ScopeLock sl(*this);
		char buffer[1024];		// the fixed size here might be a problem
		TslistDST<_key>	inx;	// new index
		ulong k, p, l;

		char *ip = cName+strlen(cName);
		strcpy(ip, ".tmp");
		
		FILE* tmp=fopen(cName, "wb");
		if(!tmp)
			return false;

		for(size_t i=0; i<cIndex.size(); i++)
		{	
			k=cIndex[i].cKey;
			p=ftell(tmp);
			l=cIndex[i].cLen;

			fseek(cDB,cIndex[i].cPos, SEEK_SET);
			fread (buffer, l,1,cDB);

			fwrite(buffer, l,1,tmp);

			inx.insert( _key(k,p,l) );
		}
		fclose(tmp);
		fclose(cDB);

		// swap databases
		// need a new buffer for renaming
		char *name = new char[1+strlen(cName)];
		memcpy(name, cName,1+strlen(cName));
		strcpy(ip, ".txt");
		::remove( cName );
		::rename( name, cName );
		delete[] name;

		// reopen the database file
		cDB = fopen(cName, "rb+");
		if(!cDB)
		{
			cDB = fopen(cName, "wb+");
			if(!cDB)
			{
				close();
				return false;
			}
		}
		// replace the index
		cIndex = inx;

		// cut the file name back to default
		*ip=0;
		return true;
	}
};
