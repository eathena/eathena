#ifndef __BASETXTDB_H__
#define __BASETXTDB_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"
#include "basearray.h"
#include "basestring.h"
#include "basefile.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// debug function.
void test_txtdb();


///////////////////////////////////////////////////////////////////////////////
/// line based text database with direct storage.
/// data sets are organized in lines, but the internal line structure is free.
/// within the overloaded checkline function the structure can be userdefined.
/// database internally holds the structure of the textfile and determines 
/// where to write data inside the file, automatically free's old saving positions
/// and saves it at new places whenever possible. write operations are immediately 
/// flushed so safe data storage is secured.
/// the textfile is locked during access to prevent external disturbances.
///
/// to allow external access, an additional file is used which can be 
/// frequently read to allow extrernal modification of the data at runtime
///
/// using "<filename>.tmp" for temporary file
/// and "<filename>.add" for the command file
///////////////////////////////////////////////////////////////////////////////
class simple_textdb : private Mutex
{
protected:
	///////////////////////////////////////////////////////////////////////////
	/// limit for length of lines.
	static const size_t line_limit;

	///////////////////////////////////////////////////////////////////////////
	/// enumeration of line types.
	enum dbline_t
	{
		LINE_COMMENT=0,	///< comment line
		LINE_EMPTY,		///< empty line (cleaned and opened for overwrite)
		LINE_VALUE		///< an real value line
	};
private:
	///////////////////////////////////////////////////////////////////////////
	/// line structure.
	/// 12/20bytes, small enough to be vectorized
	typedef struct _lines
	{
		uint32		id;
		size_t		ofs;
		size_t		len;

		// sort by len
		bool operator==(const struct _lines& l) const	{ return this->ofs==l.ofs; }
		bool operator!=(const struct _lines& l) const	{ return this->ofs!=l.ofs; }
		bool operator<=(const struct _lines& l) const	{ return this->ofs<=l.ofs; }
		bool operator< (const struct _lines& l) const	{ return this->ofs< l.ofs; }
		bool operator>=(const struct _lines& l) const	{ return this->ofs>=l.ofs; }
		bool operator> (const struct _lines& l) const	{ return this->ofs> l.ofs; }

	} lines;


	///////////////////////////////////////////////////////////////////////////
	// data
	string<>	filename;		///< the filename.
	FILE*		filehandle;		///< the file handle. (currently using stdio)
	string<>	cmd_name;		///< the command filename.
	time_t		cmd_mtime;		///< command modification time.
	bool		cleanopened;	///< clean open. aka writing to a fresh tempfile which is renamed afterwards

	vector< lines >		cmmap;	///< array of comment lines
	slist< lines >		exmap;	///< array of empty lines
	smap<uint32, lines>	idmap;	///< (simple) map of value lines


	///////////////////////////////////////////////////////////////////////////
	/// read in the file.
	/// build filestructure and clears empty marked positions in the file
	void readentries();


protected:
	///////////////////////////////////////////////////////////////////////////
	/// default constructor.
	simple_textdb();
	///////////////////////////////////////////////////////////////////////////
	/// open constructor. automatically opens the file
	simple_textdb(const string<>& name);
	///////////////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~simple_textdb()
	{
		this->close();
	}
	///////////////////////////////////////////////////////////////////////////
	/// user defined processing.
	/// needs to be overloaded in derived classes
	virtual dbline_t readline(const char* buf, uint32& id)=0;
	///////////////////////////////////////////////////////////////////////////
	/// user defined command processing.
	/// not used by default
	virtual void readcommand(const char* buf)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// looks for the external command file.
	void checkcommandfile();
	
	///////////////////////////////////////////////////////////////////////////
	/// read a specific entry.
	bool rawread(char* buf, size_t maxlen, uint32 id);
	bool rawread(string<char>& str, uint32 id);
	
	///////////////////////////////////////////////////////////////////////////
	/// write a specific entry.
	/// if possible write to the old position in file, 
	/// but will get a new entry if existing length is not sufficient.
	/// when the available write area exeeds a specific amount, 
	/// the unused potion it is cut and added to empty for reusage
	void rawwrite(const char* buf, size_t len, uint32 id);
	bool rawwrite(const string<char>& str, uint32 id);
	
	///////////////////////////////////////////////////////////////////////////
	/// remove a specific entry.
	bool rawerase(uint32 id);

public:
	///////////////////////////////////////////////////////////////////////////
	/// open file.
	/// opens (and creates if not exists) the database file,
	/// also checks for a trailing return which is mandatory
	bool open(const string<>& name);
	///////////////////////////////////////////////////////////////////////////
	/// close file.
	/// just closes the file, can be overloaded with more functionality
	/// but simple_textdb::close needs to be called explicitely then
	virtual void close();
	///////////////////////////////////////////////////////////////////////////
	/// open a already opened file in clean-mode.
	/// which creates an fresh empty database for inserting all data in order
	/// should be used in derived destructor to leave a clean file when finishing
	bool cleanopen();
	///////////////////////////////////////////////////////////////////////////
	/// closes a clean-mode file and reopens it in normal mode.
	/// take over the 
	void cleanclose();
	///////////////////////////////////////////////////////////////////////////
	/// direct access to the currently opened filename
	string<> name()	{ return this->filename; }

	uint32 getfreeid();

#if defined(DEBUG)
// for debug
size_t counts;
size_t searches;
#endif
};

template <typename T>
class typed_textdb : public simple_textdb
{
protected:
	///////////////////////////////////////////////////////////////////////////
	/// user defined processing.
	/// does some more abstraction, accepts "//" as line comment
	virtual simple_textdb::dbline_t readline(const char* buf, uint32& id)
	{
		T type;
		if( str2type(buf, type, id) )
			return simple_textdb::LINE_VALUE;
		else if( buf[0] == '\\' && buf[1] == '\\' )
			return simple_textdb::LINE_COMMENT;
		else
			return simple_textdb::LINE_EMPTY;
	}
	///////////////////////////////////////////////////////////////////////////
	/// type dependend conversion.
	virtual bool str2type(const char* buf, T& type, uint32& id)=0;
	///////////////////////////////////////////////////////////////////////////
	/// type dependend conversion.
	virtual void type2str(const T& type, string<>& str, uint32& id)=0;

public:
	typed_textdb() : simple_textdb()
	{}
	typed_textdb(const string<>& name) : simple_textdb(name)
	{}
	~typed_textdb()
	{}

	bool find(uint32 id, T& type)
	{
		string<char> str;
		return rawread(str, id) && (simple_textdb::LINE_VALUE == str2type(str, type, id));
	}
	bool save(const T& type)
	{
		string<> str;
		uint32 id;
		this->type2str(type, str, id);
		return this->rawwrite(str, id);
	}
};







///////////////////////////////////////////////////////////////////////////////
// a text based database with extra index
// not much tested though and quite basic
// but should be as fast as file access allows
// faster than sql anyway + threadsafe (not shareable though)
///////////////////////////////////////////////////////////////////////////////
#define DB_OPCOUNTMAX 4	// # of db ops before forced cache flush
class simple_database : private Mutex
{
private:
	///////////////////////////////////////////////////////////////////////////
	// index structures
	class _key
	{
	public:
		uint32 cKey1;	// key value
		uint32 cKey2;	// key2 value
		uchar cFlag;	// flag value
		ulong cPos;		// position in file
		ulong cLen;		// data length

		_key()	{}
		_key(uint32 k1, uint32 k2)
			: cKey1(k1), cKey2(k2)
		{}
		_key(uint32 k1, uint32 k2, uchar f, ulong p, ulong l)
			: cKey1(k1), cKey2(k2), cFlag(f), cPos(p), cLen(l)
		{}
		bool operator==(const _key& k) const	{ return (cKey1==k.cKey1) && (cKey2==k.cKey2); }
		bool operator> (const _key& k) const	{ return (cKey1==k.cKey1) ?  (cKey2> k.cKey2) : (cKey1> k.cKey1); }
		bool operator< (const _key& k) const	{ return (cKey1==k.cKey1) ?  (cKey2< k.cKey2) : (cKey1< k.cKey1); }
	};

private:
	///////////////////////////////////////////////////////////////////////////
	// class data
	char *cName;			// path/name of db file
	FILE *cDB;				// file handle to db
	FILE *cIX;				// file handle to index

	size_t		cOpCount;	// count of operations
	slist<_key>	cIndex;		// the index

public:
	class iterator
	{
		simple_database &cDB;
	public:
		size_t cPos;
		iterator(simple_database& sdb, bool last=false) : cDB(sdb), cPos((last)?((sdb.cIndex.size())?(sdb.cIndex.size()-1):0):0)	{}
		iterator(simple_database& sdb, const uint32 key1, const uint32 key2) : cDB(sdb)
		{
			if( !cDB.cIndex.find( simple_database::_key(key1, key2), 0, cPos) )
				cPos = cDB.cIndex.size();
		}

		iterator  operator++(int)	{ iterator temp(*this); next(); return temp; }
		iterator& operator++()		{ next(); return *this; }
		iterator  operator--(int)	{ iterator temp(*this); prev(); return temp; }
		iterator& operator--()		{ prev(); return *this;}
		bool next()					{ cPos++; return (cPos<cDB.cIndex.size()); }
		bool prev()					{ cPos--; return (cPos<cDB.cIndex.size()); }

		operator bool() const		{ return (cPos<cDB.cIndex.size()); }
		bool isValid() const		{ return (cPos<cDB.cIndex.size()); }

		uint32 Key1()				{ return isValid()?cDB.cIndex[cPos].cKey1:0; }
		uint32 Key2()				{ return isValid()?cDB.cIndex[cPos].cKey2:0; }
		uchar& Flag()				{ static uchar dummy; return isValid()?cDB.cIndex[cPos].cFlag:dummy; }

		bool find(const uint32 key1, const uint32 key2)
		{
			return cDB.cIndex.find( simple_database::_key(key1, key2), 0, cPos);
		}
		bool read(char* data, size_t maxlen)
		{
			return cDB.read(cPos, data, maxlen);
		}
	};
	friend class simple_database::iterator;
	///////////////////////////////////////////////////////////////////////////
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	simple_database() : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{}
	simple_database(const char *name) : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{
		open(name);
	}
	~simple_database()
	{
		close();
	}

	///////////////////////////////////////////////////////////////////////////
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
		checkPath(cName, name);

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
		// <# of entries> \n <i1>(0), <i2>(0), <f>(0), <p>(0), <l>(0) \n ...
		fseek(cIX, 0, SEEK_SET);
		unsigned long sz,k1,k2,f,p,l;
		if( 1==fscanf(cIX,"%lu\n", &sz) )
		{
			cIndex.realloc(sz);
			while( 5==fscanf(cIX,"%lu,%lu,%lu,%lu,%lu\n",  &k1,&k2,&f,&p,&l) )
			{
				cIndex.insert( _key(k1,k2,f,p,l) );
			}
		}

		// rebuild the database at startup might be not that expensive
		rebuild();

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
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
	///////////////////////////////////////////////////////////////////////////
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
				fflush(cDB);
			}
			if(cIX)
			{
				// write index
				fseek(cIX, 0, SEEK_SET);
				fprintf(cIX,"%lu\n", (unsigned long)cIndex.size());
				for(size_t i=0; i<cIndex.size(); ++i)
				{
					fprintf(cIX,"%lu,%lu,%lu,%lu,%lu\n",
						(unsigned long)cIndex[i].cKey1, (unsigned long)cIndex[i].cKey2,
						(unsigned long)cIndex[i].cFlag, (unsigned long)cIndex[i].cPos, (unsigned long)cIndex[i].cLen);
				}
				fflush(cIX);
			}
			// reset op counter
			cOpCount=0;
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// insert/udate
	bool insert(const uint32 key1, const uint32 key2, char* data, size_t len)
	{
		
		if(data)
		{
			ScopeLock sl(*this);
			size_t i;
			if( cIndex.find( _key(key1, key2), 0, i) )
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
				cIndex.insert( _key(key1, key2, 0, ftell(cDB), len) );
			}
			fwrite(data, 1, len, cDB);
			fflush(cDB);
			this->flush();
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// delete
	bool remove(const uint32 key1, const uint32 key2)
	{
		ScopeLock sl(*this);
		size_t pos;
		if( cIndex.find( _key(key1, key2), 0, pos) )
		{
			cIndex.removeindex(pos);
			this->flush();
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// search
	bool find(const uint32 key1, const uint32 key2, char* data, size_t maxlen) const
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		size_t i;
		if( cIndex.find( _key(key1, key2), 0, i) )
		{	
			return read(i, data, maxlen);
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// read index to buffer
	bool read(size_t inx, char* data, size_t maxlen) const
	{
		if( inx<cIndex.size() )
		{
			size_t sz = (cIndex[inx].cLen<maxlen)?cIndex[inx].cLen:maxlen;
			fseek(cDB, cIndex[inx].cPos, SEEK_SET);
			fread(data, sz, 1, cDB);
			data[sz]=0;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// gets a free index
	uint32 getfreekey()
	{	// only use keys 1..0xFFFFFFFE
		uint32 key=cIndex.size();
		if( key >= 0xFFFFFFFE )
			key=0;	// return 0 if no free key, 
		else if( !key )
			key=1;	// start with 1
		else if( key == cIndex[(size_t)(key-1)].cKey1 )
			key++;	// increment last when range is fully used
		else
		{	// find some unused key within
			size_t a=0, b=key, c;
			while( b > a+1 )
			{
				c=(a+b)/2;
				if( (cIndex[c].cKey1-cIndex[a].cKey1) > (c-a) )
					b=c;
				else
					a=c;
			}
			key = cIndex[a].cKey1 + 1;
		}
		return key;
	}
	///////////////////////////////////////////////////////////////////////////
	// rebuild database and index
	bool rebuild()
	{
		if(!cName || !cDB || !cIX)
			return false;

		ScopeLock sl(*this);
		vector<char> buffer;
		slist<_key>	inx;		// new index
		ulong k1,k2,f,p,len;

		char *ip = cName+strlen(cName);
		strcpy(ip, ".tmp");

		FILE* tmp=fopen(cName, "wb");
		if(!tmp)
			return false;

		for(size_t i=0; i<cIndex.size(); ++i)
		{
			k1=cIndex[i].cKey1;
			k2=cIndex[i].cKey2;
			f=cIndex[i].cFlag;
			p=ftell(tmp);
			len=cIndex[i].cLen;
			buffer.realloc(len);

			fseek(cDB,cIndex[i].cPos, SEEK_SET);
			fread (buffer.begin(), len,1,cDB);
			fwrite(buffer.begin(), len,1,tmp);
			inx.insert( _key(k1,k2,f,p,len) );
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
		flush(true);

		// cut the file name back to default
		*ip=0;
		return true;
	}
};
///////////////////////////////////////////////////////////////////////////////

NAMESPACE_END(basics)


#endif //__BASETXTDB_H__
