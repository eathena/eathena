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
bool scriptfile::storage::reload()
{
	scriptfile_list::iterator iter(this->files);
	for(; iter; ++iter)
	{
		if( !iter->data->load() )
			return false;
	}
	return true;
}

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
	size_t cnt=0;
	for(; iter; ++iter)
	{
		printf("%s: %i scripts\n", 
			(const char*)(iter->key),
			(int)iter->data->scripts.size());
		cnt += iter->data->scripts.size();
	}
	printf("%i files, %i scripts\n", (int)this->files.size(), (int)cnt);
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
bool scriptfile::load_file(const basics::string<>& filename)
{
	eacompiler compiler;
	return compiler.load_file(filename, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// load list of file.
bool scriptfile::load_file(const basics::vector< basics::string<> >& namelist)
{
	eacompiler compiler;
	basics::vector< basics::string<> >::iterator iter(namelist);
	bool ok = true;
	for(; ok && iter; ++iter)
	{
		ok = compiler.load_file(*iter,0);
	}
	return ok;
}

///////////////////////////////////////////////////////////////////////////////
/// load folder of file.
bool load_folder(const char* startfolder)
{
	eacompiler compiler;
	basics::file_iterator iter(startfolder, "*.ea*");
	uint cnts=0, cntb=0;
	for(; iter; ++iter)
	{
		if( basics::is_file(*iter) )
		{
			if( basics::match_wildcard("*.ea",*iter) )
			{
				if( !compiler.load_file(*iter,0) )
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
///////////////////////////////////////////////////////////////////////////////
/// get definitions
void scriptfile::get_defines(scriptdefines& defs)
{
	defs += this->definitions;
	// get definitions from parents
	filename_list::iterator iter(this->parents);
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
/// (forced) loading/reloading of this file.
bool scriptfile::load(bool forced, basics::TObjPtr<eacompiler> compiler)
{
	if( this->is_modified() || forced )
	{	
		this->parents.clear();

		if( compiler->load_file(*this, 0) )
			return false;

		// reload depending files
		filename_list::iterator iter(this->childs);
		for(; iter; ++iter)
		{
			scriptfile_ptr ptr = this->get_scriptfile(*iter);
			if( !ptr.exists() || !ptr->load(true, compiler) )
				return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// checking file state and updating the locals at the same time
bool scriptfile::is_modified()
{
	struct stat s;
	return ( 0==stat(this->c_str(), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
}

