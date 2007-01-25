// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EASTORAGE_
#define _EASTORAGE_


#include "basesafeptr.h"
#include "basevariant.h"


#include "eaprogram.h"
#include "eainstance.h"
#include "eadefine.h"
#include "eacompiler.h"

///////////////////////////////////////////////////////////////////////////////
struct scriptstorage;


///////////////////////////////////////////////////////////////////////////////
/// storage for a scriptfile.
/// contains the list of scripts and instances in that file
struct scriptfile : public basics::string<>
{
	///////////////////////////////////////////////////////////////////////////
	typedef basics::TObjPtr<scriptfile>				scriptfile_ptr;
	typedef scriptprog::script						script;
	typedef scriptinstance::instance				instance;
	
	typedef basics::slist< basics::string<> >		scriptfile_list;
	typedef basics::smap<basics::string<>,script>	script_list;
	typedef basics::vector<instance>				instance_list;

	///////////////////////////////////////////////////////////////////////////
	time_t				modtime;		///< last modification time of this file
	scriptfile_list		childs;			///< list of files that depend on this one
	scriptfile_list		parents;		///< list of files that this is depending on
	script_list			scripts;		///< list of scripts in that file
	instance_list		instances;		///< list of instances in that file
	scriptdefines		definitions;	///< defines in this file


	scriptfile() : modtime(static_cast<time_t>(-1))
	{}
	scriptfile(const basics::string<>& filename) : basics::string<>(filename), modtime(-1)
	{}
	~scriptfile()
	{}
	const basics::string<>& name() const	{ return *this; }
	///////////////////////////////////////////////////////////////////////////
	// get definitions
	void get_defines(scriptdefines& defs);
	///////////////////////////////////////////////////////////////////////////
	// (forced) loading/reloading of this file.
	bool load(bool forced=false, basics::TObjPtr<eacompiler> compiler=basics::TObjPtr<eacompiler>());

	///////////////////////////////////////////////////////////////////////////
	// checking file state and updating the locals at the same time
	bool is_modified();





	///////////////////////////////////////////////////////////////////////////
	/// script storage.
	/// contains all loaded scriptfiles
	struct storage
	{
		typedef basics::TObjPtr<scriptfile>                     scriptfile_ptr;
		typedef basics::smap<basics::string<>, scriptfile_ptr > scriptfile_list;

		scriptdefines		globaldef;			///< global definitions
		scriptfile_list		files;				///< list of files

		storage()		{}
		~storage()		{}


		bool reload();
		bool erase(const basics::string<>& filename);
		scriptfile_ptr get_scriptfile(const basics::string<>& filename) const;
		scriptfile_ptr create(const basics::string<>& filename);
		void info() const;
	};

	struct loader
	{
		basics::TObjPtr<eacompiler> compiler;

		bool process(const basics::string<>& filename, int option=0)
		{
			return compiler->load_file(filename, option);
		}
		bool process(const basics::vector< basics::string<> >& namelist, int option=0)
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

	static storage stor;	///< storage

	/// load a single file.
	static bool load_file(const basics::string<>& filename);
	/// load list of file.
	static bool load_file(const basics::vector< basics::string<> >& namelist);
	/// remove a single file.
	static bool erase_script(const basics::string<>& filename)
	{
		return scriptfile::stor.erase(filename);
	}
	/// get a scriptfile.
	static scriptfile_ptr get_scriptfile(const basics::string<>& filename)
	{
		return scriptfile::stor.get_scriptfile(filename);
	}
	static scriptfile_ptr create(const basics::string<>& filename)
	{
		return scriptfile::stor.create(filename);
	}
	
};




///////////////////////////////////////////////////////////////////////////////
#endif//_EASTORAGE_
