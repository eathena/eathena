#include "basetypes.h"
#include "baseobjects.h"
#include "basetxtdb.h"


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////
/// temporary helper. 
/// will be replaced with filestream implementation

static bool getline(vector<char>& line, FILE* file)
{
	if(file)
	{
		int c;
		//line.clear(); 
		line.resize(0); 
		//## currently better than line.clear since it also does realloc
		//## but will change when unified with caldon

		// skip leading returns
		do
		{
			c = getc(file);
		} while((c==0x0A) || (c==0x0D));
		if(c!=EOF)
		{
			//at this position c contains the first read character
			do
			{
				line.push( (char)c );		// put character to buffer
				c = getc(file);				// get next character
			} while( ((c!=0x0A) && (c!=0x0D)) && (c!=EOF) );
			// the return character is not part of the line, but skipped
			// add an eos
			line.push( '\0' );
			return true;
		}
	}
	return false;
}





///////////////////////////////////////////////////////////////////////////
/// default constructor.
simple_textdb::simple_textdb() : filehandle(NULL), cmd_mtime(0), cleanopened(false)
#if defined(DEBUG)
	, counts(0), searches(0)
#endif
{}
///////////////////////////////////////////////////////////////////////////
/// open constructor. automatically opens the file
simple_textdb::simple_textdb(const string<>& name) : filehandle(NULL), cmd_mtime(0), cleanopened(false)
#if defined(DEBUG)
	, counts(0), searches(0)
#endif
{
	this->open(name);
}

///////////////////////////////////////////////////////////////////////////////
/// read in the file.
/// build filestructure and clears empty marked positions in the file
void simple_textdb::readentries()
{
	if(filehandle)
	{
		lines tmpline;
		vector<char> line;
		char *p,*e;
		long pos;
		dbline_t typ, lasttyp=LINE_VALUE;

		ScopeLock(*this);

		exmap.clear();
		idmap.clear();

		fseek(this->filehandle,0,SEEK_SET);

		while( (-1L!=(pos=ftell(this->filehandle))) && getline(line, this->filehandle) )
		{
			tmpline.ofs = pos;
			tmpline.len = line.length();

			p=line.begin();
			e=line.begin()+tmpline.len-1; // the eos is part of length but not of string
			// cut leading whitespace
			while( stringcheck::isspace(*p) ) p++;
			// cut trailing whitespace
			while( (--e >= p) && stringcheck::isspace(*e) ) *e='\0';

			typ = this->readline(p, tmpline.id);
			
			if( typ==LINE_VALUE )
			{
				if( idmap.exists(tmpline.id) )
				{
					printf("warning: id %lu already used, ignoring previous entry\n", (ulong)tmpline.id);
					// put the previous entry to comments
					cmmap.push( idmap[tmpline.id] );
				}
				idmap[tmpline.id] = tmpline;
			}
			else if( typ==LINE_COMMENT )
			{
				cmmap.push(tmpline);
			}
			else if( typ==LINE_EMPTY )
			{
				size_t start = tmpline.ofs;
				size_t len   = tmpline.len;

				tmpline.id = 0;
				if( lasttyp==LINE_EMPTY )
				{	// two empty lines follow, take them as one
					exmap.last().len+=tmpline.len;
					
					--start;	// also clean the leading return
					++len;		// up to the trailing one
				}
				else
				{	// have a new entry
					exmap.push(tmpline);
				}

				// save original file pointer
				long safepos = ftell(this->filehandle);
				
				// clean the new empty entry
				fseek(this->filehandle, start, SEEK_SET);
				if(len) while(--len) fputc(' ', this->filehandle);

				// restore original file pointer
				fseek(this->filehandle, safepos, SEEK_SET);
			}
			// else comment
			// just don't store it at all, that way it is virtually non-existent
			lasttyp = typ;
		}
	}
	else
	{
		printf("no database open\n");
	}
}


///////////////////////////////////////////////////////////////////////////////
/// open file.
/// opens (and creates if not exists) the database file,
/// also checks for a training return which is necessary
bool simple_textdb::open(const string<>& name)
{
	ScopeLock(*this);
	if(this->filehandle)
		this->close();

	this->filename=name;
	this->cmd_name=name;
	this->cmd_name << ".add";

	if(!this->filehandle)
	{
		this->filehandle = safefopen(this->filename, "r+b");
		if(this->filehandle)
		{	// check for a trailing return
			if( 0==fseek(this->filehandle,-1,SEEK_END) && fgetc(this->filehandle)!='\n' )
			{	// and add it if not exist
				fseek(this->filehandle,0,SEEK_END);
				fputc('\n', this->filehandle);
			}
		}
		else
		{	// maybe file does not yet exist
			this->filehandle = safefopen(this->filename, "wb");
		}

		// and read in the file
		if(this->filehandle)
			this->readentries();
	}
	return (NULL!=this->filehandle);
}

///////////////////////////////////////////////////////////////////////////////
/// close file.
/// just closes the file
void simple_textdb::close()
{
	ScopeLock(*this);
	this->cleanclose();
	if(this->filehandle)
	{
		fclose(this->filehandle);
		this->filehandle = NULL;
	}
	this->filename.clear();
}

///////////////////////////////////////////////////////////////////////////////
/// open a already opened file in clean-mode.
/// this creates an fresh empty database
bool simple_textdb::cleanopen()
{
	if(this->filehandle)
	{
		ScopeLock(*this);
		string<> tmpfile(this->filename);
		tmpfile << ".tmp";
		FILE *tmphandle = safefopen(tmpfile, "wb");

		// copy comments
		vector< lines >::iterator iter(cmmap);
		vector<char> line;
		size_t len;
		for( ; iter; ++iter)
		{
			// read without the trailing return character
			len = iter->len-1;
			line.realloc(len);
			// read the data from the old file
			fseek(this->filehandle, iter->ofs, SEEK_SET);
			len = fread(line.begin(),1,len,this->filehandle);

			// reposition the entry and write the data
			iter->ofs = ftell(tmphandle);
			fwrite(line.begin(),1,len,tmphandle);
			// add a new return char
			fputc('\n',tmphandle);
			iter->len = len+1;
		}
		fclose(this->filehandle);
		this->filehandle = tmphandle;
		exmap.clear();
		idmap.clear();
	}
	return this->cleanopened=(NULL!=this->filehandle);
}

///////////////////////////////////////////////////////////////////////////////
/// closes a clean-mode file and reopens it in normal mode.
/// take over the 
void simple_textdb::cleanclose()
{
	if(this->cleanopened)
	{
		ScopeLock(*this);
		fclose(this->filehandle);
		this->filehandle = NULL;
		// take over the temp file
		file_delete(this->filename);			
		string<> tmpfile(this->filename);
		tmpfile << ".tmp";
		file_rename(tmpfile, this->filename);

		// reopen normal, no further check
		this->filehandle = safefopen(this->filename, "r+b");
		this->cleanopened = false;
	}
}

///////////////////////////////////////////////////////////////////////////
/// read a specific entry.
bool simple_textdb::rawread(char* buf, size_t maxlen, uint32 id)
{
	if( buf && this->filehandle && idmap.exists(id) )
	{
		ScopeLock(*this);
		const size_t ofs = idmap[id].ofs;
		const size_t len = min(maxlen, idmap[id].len)-1;
		size_t rlen=0;

		if(len)
		{
			fseek(this->filehandle, ofs, SEEK_SET);
			rlen = fread(buf, 1, len, this->filehandle);
		}
		buf[rlen]=0;
		// cut trailing whitespace
		char* e=buf+rlen;
		while( (--e >= buf) && stringcheck::isspace(*e) ) *e='\0';

		return (len==rlen);
	}
	return false;
}
bool simple_textdb::rawread(string<char>& str, uint32 id)
{
	if( this->filehandle && idmap.exists(id) )
	{
		ScopeLock(*this);
		const size_t ofs = idmap[id].ofs;
		const size_t len = idmap[id].len-1;
		size_t rlen=0;
		vector<char> buf;
		buf.resize(len);
		if(len)
		{
			fseek(this->filehandle, ofs, SEEK_SET);
			rlen = fread(buf.begin(), 1, len, this->filehandle);
		}
		str.assign(buf.begin(), rlen);
		str.trim();
		return (len==rlen);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// save an entry.
/// if possible write to the old position, 
/// buf get a new entry if existng length is not sufficient.
/// when the selected write area exeeds an amount, 
/// the unused potion it is cut and added to empty
void simple_textdb::rawwrite(const char* buf, size_t len, uint32 id)
{
	if(buf)
	{
		if(this->filehandle)
		{
			ScopeLock(*this);
			bool newentry=true;
			size_t p;

			if( idmap.exists(id) )
			{
				lines &ll = idmap[id];
				size_t cleanofs = ll.ofs;
				size_t cleanlen = ll.len;
				

				if( cleanlen <= len )
				{
					// check if the empty line before or after
					exmap.find(ll, 0, p);
					if( exmap.size()>0 && p<exmap.size() && p>0 && exmap[p].ofs == ll.ofs+ll.len &&
						exmap[p-1].ofs+exmap[p-1].len == ll.ofs )
					{	// line before and after this one are empty
						ll.ofs  = exmap[p-1].ofs;
						ll.len += exmap[p-1].len + exmap[p].len;
						exmap.removeindex(p);
						exmap.removeindex(p-1);
						--cleanofs;
						++cleanlen;
					}
					else if( p>0 && p<exmap.size() && exmap[p-1].ofs+exmap[p-1].len == ll.ofs )
					{	// empty line before
						ll.ofs  = exmap[p-1].ofs;
						ll.len += exmap[p-1].len;
						exmap.removeindex(p-1);
						--cleanofs;
						++cleanlen;
					}
					else if( p<exmap.size() && exmap[p].ofs == ll.ofs+ll.len )
					{	// empty line after
						ll.len += exmap[p].len;
						exmap.removeindex(p);
						++cleanlen;
					}
					else
						goto unstructured_shortcut;
				}

				if( ll.len > len )
				{	
					cleanlen = ll.len-len;

					// overwrite the old position
					fseek(this->filehandle,ll.ofs,SEEK_SET);
					fwrite(buf,len,1,this->filehandle);

					
					if( 1*(ll.len) > 2*len )
					{	// only cut off the remaining space from this entry
						// when half of it is wasted

						// add end marker directly
						fputc('\n', this->filehandle);
						++len;

						// check if the empty area can be added to following empty line
						exmap.find(ll, 0, p);
						if( p<exmap.size() && ll.ofs+ll.len == exmap[p].ofs )
						{	// the line after this one is also empty
							exmap[p].ofs  = ll.ofs + len;
							exmap[p].len += ll.len - len;
						}
						else
						{	// create a new empty entry with the rest
							lines newll;
							newll.id  = 0;
							newll.ofs = ll.ofs + len;
							newll.len = ll.len - len;
							exmap.push(newll);
							// do not overwrite the trailing return
							--cleanlen;
						}
						// and correct the len of the value node
						ll.len = len;
					}

					// and finally clean the empty potion
					if(cleanlen) while(--cleanlen) fputc(' ', this->filehandle);

					newentry=false;
				}
				else
				{
unstructured_shortcut:				
					// add the to empty list
					fseek(this->filehandle,cleanofs,SEEK_SET);
					if(cleanlen) while(--cleanlen) fputc(' ', this->filehandle);

					exmap.push(ll);

					// remove entry
					idmap.erase(id);
					// be careful from here: 
					// ll is a reference and erase has destroyed it
					// so ll is not valid any longer
				}
			}
			if(newentry)
			{
				slist< lines >::iterator iter(exmap);
				// linear time with number of empty pieces
				// log time would need an additional storage with additional maintainance
				// anyway, it is ok for the moment since amount of empty pieces is quite low
				// even on storages with >10000 entries
				// but changing it later would not affect the global behaviour
				while(iter)
				{
					if( iter->len > len )
						break;
					iter++;
#if defined(DEBUG)
					++counts;
#endif
				}
#if defined(DEBUG)
				++searches;
#endif
				if( iter && iter->len > len)
				{	// reuse this
					lines newll = *iter;

					// write the data
					fseek(filehandle,newll.ofs,SEEK_SET);
					fwrite(buf,len,1,this->filehandle);
					++len;

					if( 4*(newll.len) < 5*len )
					{	// keep the complete space inside this entry
						// when new len is less than 125% of the requested
						// (another threshold than before)

						// so remove it from empty list
						exmap.erase(iter);
					}
					else
					{	// cut it
						fputc('\n', this->filehandle);
						
						// adjust the entry inside empty list
						iter->ofs += len;
						iter->len -= len;

						// and also the new value entry
						newll.len = len;
					}

					// and store the new entry
					newll.id = id;
					idmap[id] = newll;
				}
				else
				{	// append some extra space to new entries to reduce reallocations later
					size_t reqlen = 1+((len<8*32)?len/8:32);
					lines llnew;
					fseek(this->filehandle,0,SEEK_END);
					
					llnew.ofs = ftell(this->filehandle);
					llnew.len = len+reqlen;

					fwrite(buf,len,1,this->filehandle);
					while(--reqlen) fputc(' ', this->filehandle);
					fputc('\n', this->filehandle);
					
					llnew.id  = id;						
					idmap[id] = llnew;
				}
			}
			// and flush everything to disk
			fflush(this->filehandle);
		}
		else
		{
			printf("no database open\n");
		}
	}
}
bool simple_textdb::rawwrite(const string<char>& str, uint32 id)
{
	this->rawwrite(str.begin(), str.length(), id);
	return true;
}



bool simple_textdb::rawerase(uint32 id)
{
	if(this->filehandle)
	{
		ScopeLock(*this);
		if( idmap.exists(id) )
		{
			size_t p;
			lines &ll = idmap[id];
			size_t cleanofs = ll.ofs;
			size_t cleanlen = ll.len;
			
			// check if the empty line before or after
			exmap.find(ll, 0, p);
			if( exmap.size()>0 && p<exmap.size() && p>0 && exmap[p].ofs == ll.ofs+ll.len &&
				exmap[p-1].ofs+exmap[p-1].len == ll.ofs )
			{	// line before and after this one are empty
				exmap[p-1].len += ll.len + exmap[p].len;
				exmap.removeindex(p);
				--cleanofs;
				++cleanlen;
			}
			else if( p>0 && p<exmap.size() && exmap[p-1].ofs+exmap[p-1].len == ll.ofs )
			{	// empty line before

				exmap[p-1].len +=ll.len;
				--cleanofs;
				++cleanlen;
			}
			else if( p<exmap.size() && exmap[p].ofs == ll.ofs+ll.len )
			{	// empty line after
				
				exmap[p].ofs = ll.ofs;
				exmap[p].len +=ll.len;
				exmap.removeindex(p);
				++cleanlen;
			}
			else
			{	// add new entry the to empty list
				exmap.push(ll);
			}

			// clean
			fseek(this->filehandle,cleanofs,SEEK_SET);
			if(cleanlen) while(--cleanlen) fputc(' ', this->filehandle);

			// remove entry
			idmap.erase(id);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// look/process the external command file.
void simple_textdb::checkcommandfile()
{
	struct stat s;
	FILE* cmdhandle, *tmphandle;
	if( 0==stat((const char*)this->cmd_name, &s) && s.st_mtime!=this->cmd_mtime &&
		(cmdhandle = safefopen(this->cmd_name, "rb")) )
	{	// command file has been changed
		string<> tmpname = this->cmd_name + ".tmp";

		tmphandle = safefopen(tmpname, "wb");
		if( tmphandle )
		{
			vector<char> line;
			char *p, *e;
			
			while( getline(line, cmdhandle) )
			{
				// cut leading whitespace
				p=line.begin();
				e=line.end()-1;

				while( stringcheck::isspace(*p) ) p++;
				// cut trailing whitespace
				while( (--e >= p) && stringcheck::isspace(*e) ) *e='\0';
				if( e>=p )
				{
					if(p[0]!='/' || p[1]!='/')
					{	
						// no comment, execute it
						this->readcommand(p);

						// and save it commented
						fputs("// ", tmphandle);
					}
					fputs(p, tmphandle);
					fputc('\n',tmphandle);
				}
			}
			fclose(tmphandle);
		}
		fclose(cmdhandle);

		if(tmphandle)
		{
			file_delete(this->cmd_name);
			file_rename(tmpname, this->cmd_name);

			stat((const char*)this->cmd_name, &s);
			this->cmd_mtime=s.st_mtime;
		}
	}
}


uint32 simple_textdb::getfreeid()
{
	const size_t sz = idmap.size();
	if( sz < 1 )
	{
		return sz ? (idmap.begin()->key+1) : 0;
	}
	else
	{
		const uint32 s = idmap.begin()->key;
		const uint32 e = (idmap.end()-1)->key;

		if( s+sz <= e )
		{	// search an empty position within
			smap<uint32, lines>::const_iterator iter = idmap.begin();
			size_t a=0,b=idmap.size()-1, c;
			while( a+1 < b )
			{
				c=(a+b)/2;
				if( (iter[c].key-iter[a].key) > (c-a) )
					b=c;
				else
					a=c;
			}
			return iter[a].key + 1;
		}
		else
		{	// range is full
			return e+1;
		}
	}
}














#if defined(DEBUG)

#if defined(_MSC_VER)
#pragma warning(disable : 4097) // typedef synonym to class name
#endif//_MSC_VER
typedef simple_textdb usingdb;

class example_txtdb : public usingdb
{
	typedef struct _data_t
	{
		uint32	id;
		int		inum;
		double	dnum;
		uint	unum;


		_data_t()
		{}
		_data_t(uint32 k, int i, double	d, uint u) : id(k),inum(i),dnum(d),unum(u)
		{}

		bool operator==(const struct _data_t& d) const { return id==d.id; }
		bool operator< (const struct _data_t& d) const { return id< d.id; }

		friend string<>& operator << (string<>& str, const struct _data_t d)
		{
			str << d.id << " " << d.dnum << " " << d.inum << " ";
			str.append('x', d.unum);
			return str;
		}
	} data_t;

	slist<data_t> data;
public:
	///////////////////////////////////////////////////////////////////////////
	/// default constructor.
	example_txtdb() : usingdb()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// open constructor. automatically opens the file
	example_txtdb(const string<>& name) : usingdb(name)
	{}
	///////////////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~example_txtdb()
	{
		this->close();
	}
	///////////////////////////////////////////////////////////////////////////
	/// user defined processing.
	virtual dbline_t readline(const char* buf, uint32& id)
	{
		char buffer[1024];
		int idx;
		int inum;
		double dnum;

		*buffer = 0;
		if( buf && *buf && 4== sscanf(buf, "%i %lf %i %s", &idx, &dnum, &inum, buffer) )
		{
			data.push( data_t(idx,inum,dnum,strlen(buffer)) );

			id = idx;
			return LINE_VALUE;
		}
		return LINE_EMPTY;

		//return LINE_COMMENT; // for lines with fixed position
	}
	///////////////////////////////////////////////////////////////////////////
	/// user defined command processing.
	virtual void readcommand(const char* buf)
	{
		printf("doing command: '%s'\n", buf);

	}
	///////////////////////////////////////////////////////////////////////////
	/// close db.
	virtual void close()
	{
	
		this->usingdb::close();
	}


	bool rawread(char* buf, size_t maxlen, uint32 id)
	{
		return this->usingdb::rawread(buf, maxlen,id);
	}
	
	void rawwrite(const char* buf, size_t len, uint32 id)
	{
		this->usingdb::rawwrite(buf, len,id);
	}

	void rawerase(uint32 id)
	{
		this->usingdb::rawerase(id);
	}

	void print()
	{
		slist<data_t>::iterator iter(data);

		while(iter)
		{
			printf("%lu %f %i %u\n",
				(ulong)iter->id, iter->dnum, iter->inum, iter->unum);
		
			iter++;
		}

	}
};
#endif



///////////////////////////////////////////////////////////////////////////////

void test_txtdb()
{
#if defined(DEBUG)
	{
		example_txtdb txtdb;
		string<> str;
		double res;
		string<> name = "linetester.txt";
		const char *initstr = 
			"     \n"
			"2 5.225590 18403 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
			"4 0.391578 4434 xxxxxxxxxxxxx\n"
			"3 1.250908 23071 xxxx\n"
			"  \n"
			" \n"
			"12 1.216774 23340 xxxxxxxxxxxx\n"
			"11 1.216774 23340 xxxxxxxxxxxx\n";
		const char *cmdstr = 
			"// comment   \n"
			"command 1   \n"
			"// comment\n"
			"\t\tcommand 2 \t  \n";


		FILE*file = fopen(name, "wb");
		fwrite(initstr, strlen(initstr), 1, file);
		fclose(file);
		
		txtdb.open(name);

		printf("current content:\n");
		txtdb.print();

		txtdb.rawwrite("1 1.250908 23071 xxx", 20, 1);
		txtdb.rawwrite("3 1.250908 23071 xxxxxxxxxx", 27, 3);
		txtdb.rawwrite("4 0.391578 4434 xxxx", 20, 4);

		string<> cmdname = name+".add";
		file = fopen(cmdname, "wb");
		fwrite(cmdstr, strlen(cmdstr), 1, file);
		fclose(file);


		txtdb.rawwrite("1 1.250908 23071 xxxxxxxxxxx", 28, 1);

		char buf[1024];
		size_t i,k, len, elems=1000, runs=1000000;
		ulong tick;

		// linear creation
		tick = clock();
		for(i=0; i<elems; ++i)
		{
			str.assign('x', rand()%64);
			len = sprintf(buf, "%i %f %i %s", (int)i, (double)rand()/(0.1+(double)rand()), (int)rand(), (const char*)str);
			txtdb.rawwrite(buf, len, i);
		}
		res = ((double)txtdb.counts)/((double)txtdb.searches);
		printf("db %i entries linear create => %lu\n (%f)", (int)elems, clock()-tick, res);
		

		i = txtdb.getfreeid();
		txtdb.rawerase(394);
		i = txtdb.getfreeid();



		// random access
		tick = clock();
		for(i=0; i<runs; ++i)
		{
			k = rand()%elems;
			str.assign('x', rand()%64);
			len = sprintf(buf, "%i %f %i %s", (int)k, (double)rand()/(0.1+(double)rand()), (int)rand(), (const char*)str);
			txtdb.rawwrite(buf, len, k);
		}
		printf("db %i entries %i random save => %lu\n", (int)elems, (int)runs, clock()-tick);
		printf("done %i searches with %i iterations (%f)\n", (int)txtdb.searches, (int)txtdb.counts, ((double)txtdb.counts)/((double)txtdb.searches));

		res = ((double)txtdb.counts)/((double)txtdb.searches);

		for(i=0; i<10; ++i)
		{
			k = rand()%elems;
			txtdb.rawread(buf, sizeof(buf), k);
			printf("%i: '%s'\n", (int)k, buf);
		}
		txtdb.cleanopen();

		tick = clock();
		for(i=0; i<elems; ++i)
		{
			str.assign('x', rand()%64);
			len = sprintf(buf, "%i %f %i %s", (int)i, (double)rand()/(0.1+(double)rand()), (int)rand(), (const char*)str);
			txtdb.rawwrite(buf, len, i);
		}
		printf("db %i entries linear cleansave => %lu\n", (int)elems, clock()-tick);

		txtdb.close();
	}
#endif
}



NAMESPACE_END(basics)
