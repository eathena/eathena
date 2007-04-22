// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "basefile.h"

#include "eastorage.h"

#include "eaparser.h"
#include "eacompiler.h"
#include "eaprogram.h"
#include "eadefine.h"
#include "eainstance.h"
#include "eaengine.h"

///////////////////////////////////////////////////////////////////////////////
// scriptfile::storage.


///////////////////////////////////////////////////////////////////////////////
///
bool scriptfile::storage::erase(const basics::string<>& filename)
{
	scriptfile_list::data_type* ptr = this->files.search(filename);
	if(ptr)
	{
		this->files.erase(filename);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
bool scriptfile::storage::exists(const basics::string<>& filename) const
{
	return this->files.exists(filename);
}

///////////////////////////////////////////////////////////////////////////////
///
scriptfile::scriptfile_ptr scriptfile::storage::get_scriptfile(const basics::string<>& filename) const
{
	const scriptfile_ptr* ptr = this->files.search(filename);
	return ptr?*ptr:scriptfile_ptr();
}

///////////////////////////////////////////////////////////////////////////////
///
scriptfile::scriptfile_ptr scriptfile::storage::create(const basics::string<>& filename)
{
	scriptfile_ptr& obj = this->files[filename];
	*obj = filename;
	return obj;
}

///////////////////////////////////////////////////////////////////////////////
///
void scriptfile::storage::info() const
{
	scriptfile_list::iterator iter(this->files);
	size_t scnt=0, icnt=0;
	printf("\nOverall Info:\n");
	for(; iter; ++iter)
	{
		printf("%s: %i scripts/%i instances\n", 
			(const char*)(iter->key),
			(int)iter->data->scripts.size(),
			(int)iter->data->instances.size() );
		scnt += iter->data->scripts.size();
		icnt += iter->data->instances.size();
	}
	printf("%i files, %i scripts/%i instances\n", (int)this->files.size(), (int)scnt, (int)icnt);
	
}











scriptfile::storage scriptfile::stor;



///////////////////////////////////////////////////////////////////////////////
/// get definitions
void scriptfile::get_defines(scriptdefines& defs)
{
	defs += this->definitions;
	// get definitions from parents
	name_list::iterator iter(this->parents);
	for(; iter; ++iter)
	{
		scriptfile::scriptfile_ptr ptr = scriptfile::stor.get_scriptfile(*iter);
		if( ptr.exists() )
		{
			ptr->get_defines(defs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// checking file state and updating the locals at the same time
bool scriptfile::is_modified()
{
	struct stat s;
	return ( 0==stat(this->c_str(), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
}






//## NOTE: nothing exported from main repository, 
// used a modified copy here, the export would have some more dependencies [Hinoko]
struct binstream
{
	virtual ~binstream()
	{}
	virtual void intern_put(const unsigned char v) = 0;
	virtual int intern_get()=0;
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		unsigned char* p = buffer;
		int c = this->intern_get();
		for(; c!=EOF && sz; --sz, ++p)
		{
			*p = (unsigned char)c;
			c = this->intern_get();
		}
		return p-buffer;
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		const unsigned char* p = buffer;
		if(p)
		{
			for(; sz; --sz, ++p)
				this->intern_put(*p);
		}
		return p-buffer;
	}


	template<typename T>
	void put_integer(T v, basics::bool_false)
	{	// cause compile error on non-buildin types
		void* add_your_own_stream_operator_for_this_type = v;
	}
	template<typename T>
	void put_integer(T v, basics::bool_true)
	{	// put numbers 7bit encoded with sign sorted
		const T mask = ~((T)0x3F);
		const bool is_signed = (v >> (sizeof(T)*NBBY-1));
		T x = is_signed?~v:v;
		T y = 0;
		int digit = 0;
		while( x&mask )
		{
			y = (y<<7) | (x & 0x7F);
			x >>= 7;
			++digit;
		}
		// prepare first digit
		x &= 0x3F;
		if(is_signed)
			x |= 0x40;

		for(; digit; --digit)
		{
			this->intern_put( (unsigned char)x );
			x = y & 0x7F;
			y >>= 7;
		} 
		// output last digit
		this->intern_put( (unsigned char)(x | 0x80) );
	}

	template<typename T>
	void get_integer(T& v, basics::bool_false)
	{	// cause compile error on non-buildin types
		void* add_your_own_stream_operator_for_this_type = v;
	}
	template<typename T>
	void get_integer(T& v, basics::bool_true)
	{
		 // first digit
		 unsigned char c = this->intern_get();
		 const bool is_signed = (c&0x40)!=0;
		 v = (c&0x3F);
		 while( 0==(c&0x80) )
		 {
			 c = this->intern_get();
			 v = (v<<7) | c&0x7F;
		 }
		 if( is_signed )
			 v = ~v;
	}

	template<typename T>
	void put(const T& v)
	{
		typedef typename basics::is_integral<T>::Type is_int;
		this->put_integer(v, is_int());
	}
	void put(const unsigned char v)
	{
		this->intern_put(v);
	}	
	void put(const signed char v)
	{
		this->intern_put(v);
	}
	void put(const double v)
	{
		int exponent;
		const double m = ldexp( frexp(v, &exponent), 53);	// IEEE 754 double mantissa (53bits)
		//exponent+=0x3ff; // IEEE 754 double exponent offset makes it non-negative
		const uint64 mantissa = (uint64)(floor(m));
		this->put(exponent);
		this->put(mantissa);
	}
	void put(const float v)
	{
		int exponent;
		const double m = ldexp( frexp(v, &exponent), 24);	// IEEE 754 float mantissa (53bits)
		//exponent+=0x7f; // IEEE 754 float exponent offset makes it non-negative
		const unsigned long mantissa = (unsigned long)(floor(m));
		this->put(exponent);
		this->put(mantissa);
	}
	void put(const char* v)
	{
		if(v)
		{
			for(; *v; ++v)
			{
				this->intern_put(*v);
			}
			this->intern_put(0);
		}
	}
	void put(const basics::string<>& v)
	{
		this->write((const unsigned char*)v.c_str(), 1+v.size());
	}

	template<typename T>
	void get(T& v)
	{
		typedef typename basics::is_integral<T>::Type is_int;
		this->get_integer(v, is_int());
	}
	void get(unsigned char& v)
	{
		v = this->intern_get();
	}	
	void get(signed char& v)
	{
		v = this->intern_get();
	}
	void get(double& v)
	{
		int exponent;
		uint64 mantissa;
		this->get(exponent);
		this->get(mantissa);
#if defined(_MSC_VER) && (_MSC_VER < 1300) && !defined(__ICL)
		// MSVC versions before 7 don't cast uint64 to double
		const unsigned long v0 = (unsigned long)(mantissa&0xFFFFFFFF);
		const unsigned long v1 = (unsigned long)(mantissa>>32);
		const double m = ldexp( (double)v1, 32) + v0;
#else
		const double m(mantissa);
#endif
		v = ldexp(m, -53);	// IEEE 754 double mantissa (53bits)
		v = ldexp(v, exponent); // IEEE 754 double exponent
	}
	void get(float& v)
	{
		int exponent;
		unsigned long mantissa;
		this->get(exponent);
		this->get(mantissa);
		v = ldexp((double)mantissa, -24);	// IEEE 754 float mantissa (53bits)
		v = ldexp(v, exponent); // IEEE 754 double exponent
	}
	void get(basics::string<>& v)
	{
		int c = this->intern_get();
		v.clear();
		while( c && c!=EOF )
		{
			v << ((char)c);
			c = this->intern_get();
		}
	}
};


template<typename T>
binstream& operator<<(binstream& s, const T& v)	{ s.put(v); return s; }
template<typename T>
binstream& operator>>(binstream& s, T& v)		{ s.get(v); return s; }
//## did not copy the specialization for standard types, added partially as put implements


struct binmemory : public binstream
{
	basics::vector<unsigned char> mem;

	virtual ~binmemory()
	{}

	virtual void intern_put(const unsigned char v)
	{
		this->mem.push_back(v);
	}
	virtual int intern_get()
	{
		const unsigned int v = ( this->mem.size() ) ? this->mem[0] : EOF;
		this->mem.skip(1);
		return v;
	}
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		if(buffer)
		{
			if(sz>this->mem.size()) sz = this->mem.size();
			memcpy(buffer, this->mem.begin(), sz);
			mem.skip(sz);
			return sz;
		}
		return 0;
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		if(buffer)
		{
			this->mem.append(buffer,sz);
			return sz;
		}
		return 0;
	}
};



struct binaryfile : public basics::CFile, public binstream
{
	binaryfile(const char* name, const char* mode)
	{
		this->open(name, mode);
	}

	virtual ~binaryfile()
	{}
	using binstream::put;
	//## NOTE none exported, use the modified copy here, the export would some more dependencies [Hinoko]


	virtual void intern_put(const unsigned char v)
	{
		if( this->is_open() )
		{
			putc(v, this->cFile);
		}
	}
	virtual int intern_get()
	{
		if( this->is_open() )
		{
			return getc(this->cFile);
		}
		return EOF;
	}
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		return fread(buffer,1,sz,this->cFile);
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		return fwrite(buffer,1,sz,this->cFile);
	}
};

binaryfile& operator<<(binaryfile& s, const char* v)	{ s.put(v); return s; }



enum script_tag
{
	TAG_ERR=0, 
	TAG_TAG, TAG_INT, TAG_TIME, TAG_STRING, 
	TAG_CHILD, TAG_PARENT, TAG_DEF, TAG_INSTANCE, TAG_SCRIPT, TAG_HEADER, TAG_PARAM, TAG_PROG, TAG_DATA, TAG_LABEL, TAG_PROPERTY,
	TAG_LAST
 };

binstream& operator<<(binstream& bf, const bool v)
{
	bf << ((uchar)v);
	return bf;
}
binstream& operator>>(binstream& bf, bool& v)
{
	uchar type;
	bf >> type;
	v = type;
	return bf;
}

binstream& operator<<(binstream& bf, const basics::var_t& v)
{
	bf << ((uchar)v);
	return bf;
}
binstream& operator>>(binstream& bf, basics::var_t& v)
{
	uchar type;
	bf >> type;
	v = (basics::var_t)type;
	return bf;
}

binstream& operator<<(binstream& bf, const script_tag v)
{
	const uchar x=(uchar)v; bf.put(x); return bf;
}
binstream& operator>>(binstream& bf, script_tag& v)
{
	uchar x;
	bf.get(x);
	v = ( x>=TAG_LAST )?TAG_ERR:(script_tag)x;
	return bf;
}

binstream& operator<<(binstream& bf, const basics::variant& v)
{
	bf << v.get_string() << v.type();
	return bf;
}
binstream& operator>>(binstream& bf, basics::variant& v)
{
	basics::string<> str;
	basics::var_t type;
	bf >> str >> type;
	v = str;
	v.cast(type);
	return bf;
}





///////////////////////////////////////////////////////////////////////////////
/// write to binary
bool scriptfile::to_binary(const scriptfile_ptr& file)
{
	basics::string<> name = *file;
	const size_t p = name.find_last_of('.');
	if( p!=name.npos )
		name.truncate(p);
	name+=".eab";

	binaryfile ff(name, "wb");
	if( !ff.is_open() )
		return false;


	ff << file->c_str();
	{
		ff << TAG_TIME << file->modtime;
	}
	{
		scriptfile::name_list::iterator iter(file->childs);
		for(; iter; ++iter)
		{
			ff << TAG_CHILD << *iter;
		}
	}
	{
		scriptfile::name_list::iterator iter(file->parents);
		for(; iter; ++iter)
		{
			ff << TAG_PARENT << *iter;
		}
	}
	{
		scriptdefines::iterator iter(file->definitions.table);
		for(; iter; ++iter)
		{
			ff << TAG_DEF << iter->key << iter->data;
		}
	}
	{
		scriptfile::name_list::iterator iter(file->scripts);
		for(; iter; ++iter)
		{
			scriptprog::script scr = scriptprog::get_script(*iter);
			if( scr.exists() )
			{
				ff << TAG_SCRIPT << scr->cName;

				basics::map<basics::string<>, scriptdecl>::iterator hiter(scr->cHeader);
				for(; hiter; ++hiter)
				{
					const scriptdecl& decl = hiter->data;
					ff << TAG_HEADER << decl.cName << decl.cReturn << ((uint)decl.cVarCnt) << ((uint)decl.cEntry);

					basics::vector<scriptdecl::parameter>::iterator piter(decl.cParam);
					for(; piter; ++piter)
					{
						ff << TAG_PARAM << piter->cType << piter->cConst << piter->cValue;
					}
				}

				ff << TAG_PROG << ((uint)scr->cProgramm.size());
				ff.write(scr->cProgramm.begin(), scr->cProgramm.size());
				
				ff << TAG_DATA << ((uint)scr->cDataSeg.size());
				ff.write(scr->cDataSeg.begin(), scr->cDataSeg.size());


				basics::smap<basics::string<>, uint>::iterator labeliter(scr->cLabels);
				for(; labeliter; ++labeliter)
					ff << TAG_LABEL << labeliter->key << labeliter->data;
			}
		}
	}

	{
		scriptfile::instance_list::iterator iter(file->instances);
		for(; iter; ++iter)
		{
			ff << TAG_INSTANCE << (*iter)->cType << (*iter)->cScript << (*iter)->cStart;
			scriptinstance::property::iterator prop((*iter)->cProperty);
			for(; prop; ++prop)
			{
				ff << TAG_PROPERTY << prop->key << prop->data;
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// read from binary
bool scriptfile::loader::from_binary(const basics::string<>& name, int option)
{
	binaryfile ff(name, "rb");
	if( !ff.is_open() )
	{
		fprintf(stderr, "Could not open binary file '%s'\n", name.c_str());
		return false;
	}

	script_tag tag;
	basics::string<> str;
	basics::variant var;
	scriptprog::script scr;
	scriptinstance::instance inst;
	scriptdecl* decl=NULL;
	
	ff >> str;

	if( scriptfile::exists(str) )
	{	// already loaded
		return true;
	}
	fprintf(stderr, "processing binary file '%s'\n", name.c_str());

	// create a new file entry
	scriptfile_ptr file = scriptfile::create(str);
	while( !ff.eof() )
	{

		ff >> tag;

		switch(tag)
		{
		case TAG_TIME:
		{
			ff >> file->modtime;
			break;
		}
		case TAG_CHILD:
		{
			ff >> str;
			file->childs.push_back(str);
			break;
		}
		case TAG_PARENT:
		{
			ff >> str;
			file->parents.push_back(str);
			break;
		}
		case TAG_DEF:
		{
			ff >> str >> var;
			file->definitions.insert(str,var);
			break;
		}
		case TAG_SCRIPT:
		{
			// register previous script
			if( scr.exists() )
			{
				scriptprog::regist(scr, 0);
				scr.clear();
			}
			ff >> scr->cName;
			file->scripts.push_back(scr->cName);
			break;
		}
		case TAG_HEADER:
		{
			ff >> str;
			decl = &(scr->cHeader[str]);
			decl->cName = str;
			uint c, e;
			ff >>  decl->cReturn >> c >> e;
			decl->cVarCnt = c;
			decl->cEntry  = e;
			break;
		}
		case TAG_PARAM:
		{
			scriptdecl::parameter param;
			ff >> param.cType >> param.cConst >> param.cValue;
			if( decl )
			{
				decl->cParam.push_back(param);
			}
			// error otherwise
			break;
		}
		case TAG_PROG:
		{
			uint sz;
			ff >> sz;
			scr->cProgramm.resize(sz);
			ff.read(scr->cProgramm.begin(), sz);
			break;
		}
		case TAG_DATA:
		{
			uint sz;
			ff >> sz;
			scr->cDataSeg.resize(sz);
			ff.read(scr->cDataSeg.begin(), sz);
			break;
		}
		case TAG_LABEL:
		{
			uint pos;
			ff >> str >> pos;
			scr->cLabels.insert(str,pos);
			break;
		}
		case TAG_INSTANCE:
		{
			// register last script
			if( scr.exists() )
			{
				scriptprog::regist(scr, 0);
				scr.clear();
			}

			// start a new instance
			inst.clear();
			ff >> inst->cType >> inst->cScript >> inst->cStart;

			file->instances.push_back(inst);
			break;
		}
		case TAG_PROPERTY:
		{
			ff >> str >> var;
			inst->cProperty.insert(str,var);
			break;
		}
		default:
		{	// error
			scriptfile::erase_script(*file);
			return false;
		}
		}// end switch
	}/// end while

	// register last script
	if( scr.exists() )
	{
		scriptprog::regist(scr, 0);
		scr.clear();
	}

	// load all parents
	if( file->parents.size() )
		this->load_file(file->parents, option);
	return true;

}


///////////////////////////////////////////////////////////////////////////////
/// load a single file.
bool scriptfile::loader::load_file(const basics::string<>& filename, int option)
{
	{
		basics::string<> name = filename;
		// try loading from binary first
		if( !basics::match_wildcard("*.eab", name) )
		{	// build binary extension
			const size_t p = name.find_last_of('.');
			if( p!=name.npos )
				name.truncate(p);
			name+=".eab";
		}
		if( !basics::file_exists(name) ||
			basics::file_modified(name)<basics::file_modified(filename) ||
			!this->from_binary(name, option) )
		{	
			// parse and compile when binary has failed
			if( !basics::match_wildcard("*.ea",filename) )
			{
				fprintf(stderr, "file does not use default extension\n");
			}
			if( !this->compile_file(filename,option) )
				return false;
		}
	}
	return true;

}

///////////////////////////////////////////////////////////////////////////////
/// load list of file.
bool scriptfile::loader::load_file(const basics::vector< basics::string<> >& namelist, int option)
{
	basics::vector< basics::string<> >::iterator iter(namelist);
	for(; iter; ++iter)
	{
		const basics::string<>&filename = *iter;

		basics::string<> name = filename;
		// try loading from binary first
		if( !basics::match_wildcard("*.eab", name) )
		{	// build binary extension
			const size_t p = name.find_last_of('.');
			if( p!=name.npos )
				name.truncate(p);
			name+=".eab";
		}
		if( !basics::file_exists(name) ||
			basics::file_modified(name)<basics::file_modified(filename) ||
			!this->from_binary(name, option) )
		{	
			// parse and compile when binary has failed
			if( !basics::match_wildcard("*.ea",filename) )
			{
				fprintf(stderr, "file does not use default extension\n");
			}
			if( !this->compile_file(filename,option) )
				return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
bool scriptfile::loader::load_file(const scriptfile_list& namelist, int option)
{
	scriptfile_list::iterator iter(namelist);
	for(; iter; ++iter)
	{	// only check for reloading the scripts itself here
		if( !this->compile_file(iter->key, option) )
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// load folder of file.
bool scriptfile::loader::load_folder(const char* startfolder, int option)
{
	basics::file_iterator iter(startfolder, "*.ea*", true);
	uint before=scriptcount();
	uint cnts=0;
	for(; iter; ++iter)
	{
		if( basics::is_file(*iter) )
		{
			if( basics::match_wildcard("*.ea",*iter) )
			{
				basics::string<> name = *iter;
				name+='b';
				if( basics::file_exists(name) && 
					basics::file_modified(name)>basics::file_modified(*iter) &&
					this->from_binary(name,option) )
				{
					//
				}
				else if( this->compile_file(*iter,option) )
				{
					++cnts;
				}
				else
				{
					return false;
				}
				
			}
			else if( basics::match_wildcard("*.eab",*iter) )
			{
				if( !this->from_binary(*iter, option) )
				{
					return false;
				}
			}
		}
	}
	printf("loaded %u new scripts and %u compiled images\n", cnts, (uint)(scriptcount()-before-cnts));
	return true;
}
