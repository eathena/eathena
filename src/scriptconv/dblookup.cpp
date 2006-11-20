
#include "dblookup.h"
#include "basearray.h"

template<typename T>
struct storage
{
	basics::smap<basics::string<>, T*>	id2obj;
	basics::smap<basics::string<>, T*>	name2obj;

	~storage()
	{
		typename basics::smap<basics::string<>, T*>::iterator iter(id2obj);
		for(; iter; ++iter)
			delete iter->data;
		id2obj.clear();
		name2obj.clear();
	}
	size_t load(const char* filename);
	bool insert(const T& me);
	const T* lookup(basics::string<> str) const
	{	// link problems in msvc when declared outside the template body
		// strip quotes
		if( str[0]=='\"' )
			str.truncate(1, str.size()>2?str.size()-2:0);

		if( this->id2obj.exists(str) )
			return this->id2obj[str];
		if( this->name2obj.exists(str) )
			return this->name2obj[str];
		return NULL;
	}
};

template<typename T>
size_t storage<T>::load(const char* filename)
{	// conversion fileformat is:
	// lines with
	// id,name[,.*]
	FILE* fp =fopen(filename, "rb");
	if( !fp )
	{
		fprintf(stderr, "cannot open %s\n", filename);
	}
	else
	{
		char line[1024], *ip, *kp;
		basics::string<> id, name;

		while(fgets(line, sizeof(line), fp))
		{
			// terminate buffer
			line[sizeof(line)-1] = '\0';

			// skip leading spaces
			ip = line;
			while( basics::stringcheck::isspace(*ip) ) ip++;

			// check for comments (only "//") 
			// does not check for escapes or string markers 
			// as a appropiate config grammer needs to be defined first
			for(kp=ip; *kp; ++kp)
			{	// cut of trailing comments/newlines
				if(kp[0]=='\r' || kp[0]=='\n' || (kp[0]=='/' && kp[1]=='/') )
				{
					kp[0] = 0;
					break;
				}
			}

			// skipping empty lines
			if( !ip[0] )
				continue;

			T me;
			for(me.ID.clear(); *ip && *ip!=','; ++ip)
			{
				me.ID << *ip;
			}
			me.ID.trim();

			if(*ip) ++ip;

			if( !*ip )
				continue;

			for(me.Name1.clear(); *ip && *ip!=','; ++ip)
			{
				me.Name1 << *ip;
			}
			me.Name1.trim();

			me.conv_only=true;
			this->insert(me);
		}
	}
	return this->id2obj.size();
}

template<typename T>
bool storage<T>::insert(const T& me)
{
	if( this->id2obj.exists(me.ID) )
	{
		T* ptr = this->id2obj[me.ID];
		if( ptr->conv_only && !me.conv_only 
			&& ptr->Name1==me.Name1)
		{	// overwrite the conversion entry
			*ptr = me;
			return true;
		}
		else
		{
			fprintf(stderr, "%s/%s already exists\n", 
				(const char*)me.ID, (const char*)me.Name1);
		}
	}
	else if( this->id2obj.exists(me.Name1) )
	{
		fprintf(stderr, "%s/%s name duplication\n", 
			(const char*)me.ID, (const char*)me.Name1);
	}
	else
	{	// name and id unknown
		T* ptr = new T(me);
		this->id2obj[me.ID] = ptr;
		this->name2obj[me.Name1] = ptr;
		return true;
	}
	return false;
}




///////////////////////////////////////////////////////////////////////////////

storage<itemdb_entry> itemdb;
storage<mobdb_entry> mobdb;
storage<npcdb_entry> npcdb;

///////////////////////////////////////////////////////////////////////////////
void itemdb_entry::load(const char* filename)
{
	fprintf(stderr, "loading conversion itemdb... ");
	fprintf(stderr, "%i entries\n", (int)itemdb.load(filename));
}
bool itemdb_entry::insert(const itemdb_entry& me)
{
	return itemdb.insert(me);
}
const itemdb_entry* itemdb_entry::lookup(const basics::string<>& str)
{
	return itemdb.lookup(str);
}
///////////////////////////////////////////////////////////////////////////////
void mobdb_entry::load(const char* filename)
{
	fprintf(stderr, "loading conversion mobdb... ");
	fprintf(stderr, "%i entries\n", (int)mobdb.load(filename));
}
bool mobdb_entry::insert(const mobdb_entry& me)
{
	return mobdb.insert(me);
}
const mobdb_entry* mobdb_entry::lookup(const basics::string<>& str)
{
	return mobdb.lookup(str);
}
///////////////////////////////////////////////////////////////////////////////
void npcdb_entry::load(const char* filename)
{
	fprintf(stderr, "loading conversion npcdb... ");
	fprintf(stderr, "%i entries\n", (int)npcdb.load(filename));
}
bool npcdb_entry::insert(const npcdb_entry& me)
{
	return npcdb.insert(me);
}
const npcdb_entry* npcdb_entry::lookup(const basics::string<>& str)
{
	return npcdb.lookup(str);
}

