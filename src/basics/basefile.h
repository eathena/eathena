#ifndef __BASEFILE_H__
#define __BASEFILE_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basetime.h"
#include "basearray.h"
#include "basestring.h"

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// seek defines do not always exist
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif


///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define PATHSEP '\\'
#else/////////////////////////
#define PATHSEP '/'
#endif////////////////////////

///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#define PATH_SPLITTER ";"
#else////////////////////////
#define PATH_SPLITTER ":"
#endif////////////////////////


////////////////////////////////////////////////////////////////////////////////
/// basic file system functions
bool is_console(FILE* file);
bool is_present(const char*name);
bool is_folder(const char*name);
bool is_file(const char*name);


////////////////////////////////////////////////////////////////////////////////
/// file functions
bool file_exists(const char*name);
unsigned long filesize(const char*name);
bool file_readable(const char*name);
bool file_writable(const char*name);
bool file_delete(const char*name);
bool file_rename (const char* old_name, const char* new_name);
bool file_move (const char* old_name, const char* new_name);
bool file_copy (const char* old_name, const char* new_name);


////////////////////////////////////////////////////////////////////////////////
/// taken from stlplus
/// Read-only versus read-write control. This is equivalent to chmod on Unix,
/// but I've insulated the user from the low-level routine because of
/// differences in the OSs' interpretation of the mode parameter. I've also
/// defined a new set of constants to control this, again because of
/// inconsistencies. The idea is to combine the constants as bit-masks so as to
/// build up a set of permissions. The modes are ORed together to build up a
/// set of permissions and then ANDed with a mask to control which people have
/// that permission. Permissions can be ORed together too. So, for example, to
/// give the owner read-write access and all others only read access, you would
/// use the expression:
///   ((read_mode | write_mode) & owner_mask) | (read_mode & (group_mask | other_mask))
/// This can be simplified by using combined modes and combined masks to:
///   (read_write_mode & owner_mask) | (read_mode & non_owner_mask)

// basic modes
extern const int read__mode;
extern const int write_mode;
extern const int execute_mode;
// combined modes
extern const int none_mode;
extern const int read_write_mode;
extern const int all_mode;
// basic users
extern const int owner_mask;
extern const int group_mask;
extern const int other_mask;
// combined users
extern const int non_owner_mask;
extern const int all_mask;
// common settings
extern const int read_mode_all;
extern const int read_write_mode_owner_read_mode_all;
extern const int read_mode_owner_only;
extern const int read_write_mode_owner_only;
// the function itself
bool file_set_mode(const char* name, int mode);

/// get the file's time stamps as a time_t
time_t file_created(const char* filename);
time_t file_modified(const char* filename);
time_t file_accessed(const char* filename);

// build filenames
string<> create_filespec(const char* foldername, const char* filename);
string<> create_filespec(const char* foldername, const char* basename, const char* extension);
string<> create_filename(const char* basename, const char* extension);

////////////////////////////////////////////////////////////////////////////////
/// folder functions
bool folder_create(const char* folder);
bool folder_exists(const char* folder);
bool folder_readable(const char* folder);
bool folder_writable(const char* folder);
bool folder_delete(const char* folder, bool recurse = false);
bool folder_rename (const char* old_directory, const char* new_directory);
bool folder_empty(const char* folder);

bool folder_set_current(const char* folder);
string<> folder_current(void);
string<> folder_current_full(void);
string<> folder_home(void);
string<> folder_down(const char* folder, const char* subfolder);
string<> folder_up(const char* folder, unsigned levels = 1);

vector< string<> > folder_subdirectories(const char* folder);
vector< string<> > folder_files(const char* folder);
vector< string<> > folder_all(const char* folder);
vector< string<> > folder_wildcard(const char* folder, const char* wildcard, bool subdirs = true, bool files = true);

////////////////////////////////////////////////////////////////////////////////
/// path functions
bool is_full_path(const char* path);
bool is_relative_path(const char* path);

/// convert to a full path relative to the root path
string<> folder_to_path(const char* root, const char* folder);
string<> filespec_to_path(const char* root, const char* filespec);

/// convert to a full path relative to the current working directory
string<> folder_to_path(const char* folder);
string<> filespec_to_path(const char* filespec);

/// convert to a relative path relative to the root path
string<> folder_to_relative_path(const char* root, const char* folder);
string<> filespec_to_relative_path(const char* root, const char* filespec);

/// convert to a relative path relative to the current working directory
string<> folder_to_relative_path(const char* folder);
string<> filespec_to_relative_path(const char* filespec);

/// append a folder separator to the path to make it absolutely clear that it is a folder
string<> folder_append_separator(const char* folder);

////////////////////////////////////////////////////////////////////////////////
// access functions split a filespec into its elements
string<> basename_part(const char* filespec);
string<> filename_part(const char* filespec);
string<> extension_part(const char* filespec);
string<> folder_part(const char* filespec);

/// split a path into a vector of elements - i.e. split at the folder separator
vector< string<> > folder_elements(const char* folder);
vector< string<> > filespec_elements(const char* filespec);

////////////////////////////////////////////////////////////////////////////////
/// Path lookup. lookup normally carried out by the shell to find a command in a
/// directory in the PATH. Give this function the name of a command and it
/// will return the full path. It returns an empty string on failure.
string<> path_lookup (const char* command);

/// Generalised form of the above, takes a second argument
/// - the list to search. This can be used to do other path lookups,
/// such as LD_LIBRARY_PATH. The third argument specifies the splitter -
/// the default value of PATH_SPLITTER is appropriate for environment variables.
string<> lookup (const char* file, const char* path, const char* splitter = PATH_SPLITTER);

/// utility function for finding the folder that contains the current executable
/// the argument is the argv[0] parameter passed to main
string<> install_path(const char* argv0);

////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
/// basic path checkings (check for wrong pathseperators)
char* checkPath(char *path, size_t sz, const char*srcpath);
char* checkPath(char *path, const char*srcpath);
char* checkPath(char *path);

///////////////////////////////////////////////////////////////////////////////
/// fopen replacement, 
inline FILE* safefopen(const char*name, const char*option)
{	// windows MAXPATH is 260, unix is longer
	char	 namebuf[2048];
	checkPath(namebuf, sizeof(namebuf), name);
	return fopen( namebuf, option);
}




///////////////////////////////////////////////////////////////////////////////
/// basic callback object.
///////////////////////////////////////////////////////////////////////////////
class CFileProcessor : public noncopyable
{
public:
	CFileProcessor()			{}
	virtual ~CFileProcessor()	{}
	virtual bool process(const char *name) const = 0;
};

bool findFiles(const char *p, const char *pat, const CFileProcessor& fp);
bool findFiles(const char *p, const char *pat, void (func)(const char*) );




//////////////////////////////////////////////////////////////////////////
/// file class.
/// behaves like FILE with OO extension
/// supports only signed 32bit addressing, so filesize is 2GB max 
/// though it can use 64bit on 64 unix
//## check merging with file streams
//////////////////////////////////////////////////////////////////////////
class CFile : public global, public noncopyable
{
	FILE *cFile;
public:
	CFile() : cFile(NULL)
	{}
	CFile(const char* name, const char* mode="rb") : cFile(NULL)
							{ open(name, mode); }
	~CFile()				{ close(); }
	bool ok()				{ return (cFile!=NULL); }

	bool is_open()			{ return (cFile!=NULL); }
	bool clear()			{ return true; }

	bool open(const char* name, const char* mode="rwb")
	{
		if(cFile)	close();
		if(name)	cFile=safefopen(name, mode);
		return is_open();
	}
	bool close()
	{
		if(cFile)
		{
			fclose(cFile); 
			cFile=NULL;
		}
		return true;
	}

	size_t filesize()	
	{
		long end, pos = ftell(cFile);
		fseek(cFile, 0, SEEK_END);
		end = ftell(cFile);
		fseek(cFile, pos, SEEK_SET);
		return end;
	}
	size_t size()			{ return filesize(); }

	bool eof()
	{
		if(cFile)
		{
			long end, pos = ftell(cFile);
			fseek(cFile, 0, SEEK_END);
			end = ftell(cFile);
			fseek(cFile, pos, SEEK_SET);
			return pos>=end;
		}
		// if no file we are eof
		return true;
	}

	FILE* operator()()	{ return cFile; }
	operator FILE*()	{ return cFile; }

	ssize_t getline(char *buf, size_t maxlen);
	ssize_t getline(string<>& str);
	ssize_t writeline(const char *buf, size_t maxlen=0);
	ssize_t writeline(const string<>& str);
	
	long getpos()			
	{
		if(cFile) 
			return ftell(cFile);
		return 0;
	}
	bool setpos(long pos)
	{
		if(cFile)
			return 0==fseek(cFile,pos, SEEK_SET); 
		return false;
	}
	long tellg()			{ return getpos(); }
	bool seekg(long pos)	{ return setpos(pos); }


	friend CFile& operator <<( CFile& cf, const char* str)
	{
		if( cf.is_open() && str )
		{
			while( *str ) putc(uchar(*str++), cf.cFile);
		}
		return cf;
	}

};


NAMESPACE_END(basics)


#endif //__BASEFILE_H__
