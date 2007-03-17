// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EASTORAGE_
#define _EASTORAGE_


#include "basesafeptr.h"
#include "basefile.h"
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
	/// script storage.
	/// contains all loaded scriptfiles
	struct storage
	{
		typedef basics::TObjPtrCount<scriptfile>                scriptfile_ptr;
		typedef basics::smap<basics::string<>, scriptfile_ptr > scriptfile_list;

		scriptdefines		globaldef;			///< global definitions
		scriptfile_list		files;				///< list of files

		storage()		{}
		~storage()		{}

		bool erase(const basics::string<>& filename);
		scriptfile_ptr get_scriptfile(const basics::string<>& filename) const;
		scriptfile_ptr create(const basics::string<>& filename);
		void info() const;
	};

	///////////////////////////////////////////////////////////////////////////
	typedef storage::scriptfile_ptr					scriptfile_ptr;
	typedef storage::scriptfile_list				scriptfile_list;
	typedef scriptprog::script						script;
	typedef scriptinstance::instance				instance;
	
	typedef basics::slist< basics::string<> >		name_list;
	typedef basics::vector<script>					script_list;
	typedef basics::vector<instance>				instance_list;

	///////////////////////////////////////////////////////////////////////////
	time_t				modtime;		///< last modification time of this file
	name_list			childs;			///< list of files that depend on this one
	name_list			parents;		///< list of files that this is depending on
	name_list			scripts;		///< list of scripts in that file
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
//	bool load(bool forced=false, basics::TObjPtr<eacompiler> compiler=basics::TObjPtr<eacompiler>());

	///////////////////////////////////////////////////////////////////////////
	// checking file state and updating the locals at the same time
	bool is_modified();



	static storage stor;	///< storage

	/// load a single file.
	static bool load_file(const basics::string<>& filename, int option=0);
	/// load list of file.
	static bool load_file(const basics::vector< basics::string<> >& namelist, int option=0);
	/// load list of file.
	static bool load_file(const scriptfile_list& namelist, int option=0);
	
	/// load a folder of file.
	static bool load_folder(const char* startfolder, int option=0);
	/// remove a single file.
	static bool erase_script(const basics::string<>& filename)
	{
		return scriptfile::stor.erase(filename);
	}
	/// remove a single file.
	static void info()
	{
		scriptfile::stor.info();
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
	static bool to_binary(const scriptfile_ptr& file);
	static bool from_binary(const basics::string<>& name);
};




///////////////////////////////////////////////////////////////////////////////
#endif//_EASTORAGE_
