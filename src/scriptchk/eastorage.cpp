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
		delete ptr;
	}
	return true;
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











///////////////////////////////////////////////////////////////////////////////
// scriptfile.
/*
	struct loader
	{
		basics::TObjPtr<eacompiler> compiler;

		bool load_folder(const char* startfolder)
		{
			basics::file_iterator iter(startfolder, "*.ea*");
			uint cnts=0, cntb=0;
			for(; iter; ++iter)
			{
				if( basics::is_file(*iter) )
				{
					if( basics::match_wildcard("*.ea",*iter) )
					{
						if( !this->load_file(*iter) )
							return false;
						++cnts;
					}
					else if( basics::match_wildcard("*.eab",*iter) )
					{
						//if( !this->load_binaryfile(*iter) )
						//	return false;
						//++cntb;
					}
				}
			}
			printf("loaded %u new scripts and %u compiled images\n", cnts, cntb);
			return true;
		}
		bool load_file(const basics::string<>& filename, int option=0)
		{
			return compiler->load_file(filename, option);
		}
		bool load_file(const basics::vector< basics::string<> >& namelist, int option=0)
		{
			eacompiler& comp = *this->compiler;
			basics::vector< basics::string<> >::iterator iter(namelist);
			bool ok = true;
			for(; ok && iter; ++iter)
			{
				ok = comp.load_file(*iter,0);
			}
			return ok;
		}
	};
*/
scriptfile::storage scriptfile::stor;

///////////////////////////////////////////////////////////////////////////////
/// load a single file.
bool scriptfile::load_file(const basics::string<>& filename, int option)
{
	eacompiler compiler;
	basics::string<> name = filename;
	const size_t p = name.find_last_of('.');
	if( p!=name.npos )
		name.truncate(p);
	name+=".eab";
	return ( basics::file_exists(name) &&
			basics::file_modified(name)>basics::file_modified(filename) &&
			scriptfile::from_binary(name)) ||
			compiler.load_file(filename, option);
}

///////////////////////////////////////////////////////////////////////////////
/// load list of file.
bool scriptfile::load_file(const basics::vector< basics::string<> >& namelist, int option)
{
	eacompiler compiler;
	basics::vector< basics::string<> >::iterator iter(namelist);
	bool ok = true;
	for(; ok && iter; ++iter)
	{
		basics::string<> name = *iter;
		const size_t p = name.find_last_of('.');
		if( p!=name.npos )
			name.truncate(p);
		name+=".eab";
		ok = ( basics::file_exists(name) &&
			basics::file_modified(name)>basics::file_modified(*iter) &&
			scriptfile::from_binary(name)) ||
			compiler.load_file(*iter, option);
	}
	return ok;
}

///////////////////////////////////////////////////////////////////////////////
///
bool scriptfile::load_file(const scriptfile_list& namelist, int option)
{
	eacompiler compiler;
	scriptfile_list::iterator iter(namelist);
	for(; iter; ++iter)
	{	// only check for reloading the scripts itself here
		if( !compiler.load_file(iter->key, option) )
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// load folder of file.
bool scriptfile::load_folder(const char* startfolder, int option)
{
	eacompiler compiler;
	basics::file_iterator iter(startfolder, "*.ea*", true);
	uint cnts=0, cntb=0;
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
					scriptfile::from_binary(name) )
					++cntb;
				else if( !compiler.load_file(*iter,option) )
					return false;
				++cnts;
			}
			else if( basics::match_wildcard("*.eab",*iter) )
			{
				if( !scriptfile::from_binary(*iter) )
					return false;
				++cntb;
			}
		}
	}
	printf("loaded %u new scripts and %u compiled images\n", cnts, cntb);
	return true;
}

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

///////////////////////////////////////////////////////////////////////////////
/// write to binary
bool scriptfile::to_binary(const scriptfile_ptr& file)
{
	basics::string<> name = *file;
	const size_t p = name.find_last_of('.');
	if( p!=name.npos )
		name.truncate(p);
	name+=".eab";

	/*
	basics::fostream str(name);
	str << file->childs;
	str << file->parents;
	str << file->definitions.table;
	str << scripts;
	//str << file->instances;
	*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// read from binary
bool scriptfile::from_binary(const basics::string<>& name)
{
	/*
	basics::fistream str(name);
	str >> file->childs;
	str >> file->parents;
	str >> file->definitions.table;
	str >> scripts;
	//str >> file->instances;
	*/
	fprintf(stderr, "loading compiled images is not yet supported\n");
	return false;
}
