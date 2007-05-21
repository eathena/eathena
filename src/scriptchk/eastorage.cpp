// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "basebinstream.h"

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
	obj->assign(filename);
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








enum script_tag
{
	TAG_ERR=0, 
	TAG_TAG, TAG_INT, TAG_TIME, TAG_STRING, 
	TAG_CHILD, TAG_PARENT, TAG_DEF, TAG_INSTANCE, TAG_SCRIPT, TAG_HEADER, TAG_PARAM, TAG_PROG, TAG_DATA, TAG_LABEL, TAG_PROPERTY,
	TAG_LAST
 };

basics::binstream& operator<<(basics::binstream& bf, const bool v)
{
	bf << ((uchar)v);
	return bf;
}

basics::binstream& operator>>(basics::binstream& bf, bool& v)
{
	uchar type=0;
	bf >> type;
	v = type;
	return bf;
}

basics::binstream& operator<<(basics::binstream& bf, const basics::var_t& v)
{
	bf << ((uchar)v);
	return bf;
}
basics::binstream& operator>>(basics::binstream& bf, basics::var_t& v)
{
	uchar type=0;
	bf >> type;
	v = (basics::var_t)type;
	return bf;
}

basics::binstream& operator<<(basics::binstream& bf, const script_tag v)
{
	const uchar x=(uchar)v; bf.put(x); return bf;
}
basics::binstream& operator>>(basics::binstream& bf, script_tag& v)
{
	uchar x=0;
	bf.get(x);
	v = ( x>=TAG_LAST )?TAG_ERR:(script_tag)x;
	return bf;
}

basics::binstream& operator<<(basics::binstream& bf, const basics::variant& v)
{
	bf << v.get_string() << v.type();
	return bf;
}
basics::binstream& operator>>(basics::binstream& bf, basics::variant& v)
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

	basics::binaryfile ff(name, "wb");
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
	basics::binaryfile ff(name, "rb");
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
			uint sz=0;
			ff >> sz;
			scr->cProgramm.resize(sz);
			ff.read(scr->cProgramm.begin(), sz);
			break;
		}
		case TAG_DATA:
		{
			uint sz=0;
			ff >> sz;
			scr->cDataSeg.resize(sz);
			ff.read(scr->cDataSeg.begin(), sz);
			break;
		}
		case TAG_LABEL:
		{
			uint pos=0;
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
