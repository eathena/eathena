#include "basetypes.h"
#include "baseobjects.h"
#include "basefile.h"
#include "basestrsearch.h"

///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// do not include this globally, it poisons the namespace
#include <direct.h>
#include <io.h>			// for access
						// and also the modi are not defined in windows
#define F_OK	0x0		// Existence only
#define X_OK	0x1		// Exec permission
#define W_OK	0x2		// Write permission
#define R_OK	0x4		// Read permission
#define RW_OK	0x6		// Read and write permission

///////////////////////////////////////////////////////////////////////////////
#else

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


#ifndef MAX_PATH
#define MAX_PATH 2048
#endif

#endif
///////////////////////////////////////////////////////////////////////////////



NAMESPACE_BEGIN(basics)


bool is_console(FILE* file)
{
	return 0!=isatty(fileno(file));
}


///////////////////////////////////////////////////////////////////////////////
// inconvinient/inefficient/slow
// but working completely platform/implementation independend
ssize_t CFile::getline(char *buf, size_t maxlen)
{
	char *ip = buf;
	int c;
	if(cFile)
	{
		// skip all leading returns
		do
		{
			c = getc(cFile);
		}while((c==0x0A) || (c==0x0D));
		if(c==EOF)
			return -1;
		//at this position c contains the first read character
		do
		{
			if(ip < buf+maxlen-1)		// if buffer not full
				*ip++ = (char)c;		// put character to buffer
			c = getc(cFile);			// get next character
		}while( ((c!=0x0A) && (c!=0x0D)) && (c!=EOF) );
		*ip=0;							// put EOS
		// return the number of chars in the buffer
		return ip-buf;
	}
	*buf=0;
	return -1;
}

ssize_t CFile::getline(string<>& str)
{
	int c;
	str.clear();
	if(cFile)
	{
		// skip leading returns
		do
		{
			c = getc(cFile);
		} while((c==0x0A) || (c==0x0D));
		if(c==EOF)
			return -1;
		//at this position c contains the first read character
		do
		{
			str.append( (char)c );		// put character to buffer
			c = getc(cFile);			// get next character
		}while( ((c!=0x0A) && (c!=0x0D)) && (c!=EOF) );
		// return the number of chars in the buffer
		return str.length();
	}
	return -1;
}
ssize_t CFile::writeline(const char *buf, size_t maxlen)
{
	if(buf && cFile)
	{
		if(maxlen==0) maxlen= hstrlen(buf);
		ssize_t sz=fwrite(buf,1,maxlen,cFile);
		if( buf[maxlen-1] != '\n')
			sz+=fwrite("\n",1,1,cFile);
		return sz;
	}
	return -1;
}
ssize_t CFile::writeline(const string<>& str)
{
	if(cFile)
	{
		ssize_t sz=fwrite((const char*)str,1,str.size(),cFile);
		if( str[str.size()-1] != '\n' )
			sz+=fwrite("\n",1,1,cFile);
		return sz;
	}
	return -1;
}



///////////////////////////////////////////////////////////////////////////////
// copies from scpath to path and exchange the path seperator
// no check for buffer overflows
char* checkPath(char *path, size_t sz, const char *srcpath)
{	// just make sure the char*path is not const
	if( path )
	{
		char *p=path, *ep=path+sz-1;
		if( srcpath )
		{
			while(*srcpath && p<ep)
			{
#ifdef WIN32
				if(*srcpath=='/') {
					*p++ = '\\';
#else
				if(*srcpath=='\\') {
					*p++ = '/';
#endif
					srcpath++;
				}
				else
					*p++ = *srcpath++;
			}
		}
		*p = 0; //EOS
	}
	return path;
}
char* checkPath(char *path, const char *srcpath)
{	// just make sure the char*path is not const
	if( path )
	{
		char *p=path;
		if( srcpath )
		{
			while(*srcpath) {
#ifdef WIN32
				if(*srcpath=='/') {
					*p++ = '\\';
#else
				if(*srcpath=='\\') {
					*p++ = '/';
#endif
					srcpath++;
				}
				else
					*p++ = *srcpath++;
			}		
		}
		*p = *srcpath; //EOS
	}
	return path;
}
// just exchange the path seperator
char* checkPath(char *path)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path)
	while(*p) {
#ifdef WIN32
		if(*p=='/') {
			*p++ = '\\';
#else
		if(*p=='\\') {
			*p++ = '/';
#endif
		}
		else
			p++;
	}
	return path;
}


///////////////////////////////////////////////////////////////////////////////
// runs recursively through the directories starting at the given path
// calling the FileProcessor with each encountered file name containing the given pattern
bool findFiles(const char *p, const char *pat, const CFileProcessor& fp)
{	
#ifdef WIN32
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	bool ok=true;
	
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;
	
	checkPath(tmppath,path);
	if( PATHSEP != tmppath[hstrlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");
	
	hFind = FindFirstFile(tmppath, &FindFileData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if(strcmp(FindFileData.cFileName, "..") == 0)
				continue;
			// exceeded pathlength
			if( 0>snprintf(tmppath, sizeof(tmppath), "%s%c%s",path,PATHSEP,FindFileData.cFileName) )
				continue;

			// give name to processor
			if(FindFileData.cFileName && match_wildcard<char>(pattern, FindFileData.cFileName))
				ok = fp.process( tmppath );
			// and descend to subdirs
			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				ok=findFiles(tmppath, pat, fp);

		}while( ok && FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
   }
   return ok;
#else
	bool ok=true;
	DIR* dir;					// pointer to the scanned directory.
	struct dirent* entry;		// pointer to one directory entry.
	struct stat dir_stat;       // used by stat().
	char tmppath[MAX_PATH+1];
	char path[MAX_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL)
	{
		strncpy(path,p,sizeof(path)); //"If count is less than or equal to the length of strSource, a null character is not appended automatically to the copied string."
		path[MAX_PATH]=0;
	}

	// open the directory for reading
	dir = opendir( checkPath(path, path) );
	if(!dir)
	{
		fprintf(stderr, "Cannot read directory '%s'\n", path);
		return false;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while( ok && (entry = readdir(dir)) )
	{	// skip the "." and ".." entries.
		if(strcmp(entry->d_name, ".") == 0)
			continue;
		if(strcmp(entry->d_name, "..") == 0)
			continue;

		// exceeded pathlength
		if( 0>snprintf(tmppath, sizeof(tmppath), "%s%c%s",path, PATHSEP, entry->d_name) )
			continue;

		// check if the pattern matchs.
		if( entry->d_name && match_wildcard<char>(pattern, entry->d_name) )
			ok = fp.process( tmppath );

		// check if it is a directory.
		if( stat(tmppath, &dir_stat) == -1)
		{
			fprintf(stderr, "stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if( S_ISDIR(dir_stat.st_mode) )
		{
			// decent recursivly
			ok = findFiles(tmppath, pat, fp);
		}
	}//end while
	closedir(dir);
	return ok;
#endif
}



///////////////////////////////////////////////////////////////////////////////
// using the processing class parameter
// with the FileProcessor object
///////////////////////////////////////////////////////////////////////////////

class CSimpleFileProcessor  : public CFileProcessor
{
	void (*func)(const char*);
public:
	CSimpleFileProcessor( void (*f)(const char*) ) : func(f)	{}
	virtual ~CSimpleFileProcessor()	{}
	virtual bool process(const char *name) const
	{
		func(name);
		return true;
	}
};

bool findFiles(const char *p, const char *pat, void (func)(const char*) )
{
	return findFiles(p,pat, CSimpleFileProcessor(func) );
}

///////////////////////////////////////////////////////////////////////////////
// or have it directly
///////////////////////////////////////////////////////////////////////////////
/*

bool findFiles(const char *p, const char *pat, void (func)(const char*) )
{	
#ifdef WIN32
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;
	
	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");
	
	hFind = FindFirstFile(tmppath, &FindFileData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if(strcmp(FindFileData.cFileName, "..") == 0)
				continue;

			snprintf(tmppath, sizeof(tmppath), "%s%c%s",path,PATHSEP,FindFileData.cFileName);

			if(FindFileData.cFileName && match_wildcard<char>(pattern, FindFileData.cFileName)) {
				func( tmppath );
			}


			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				findfile(tmppath, pat, func);
			}
		}while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
   }
   return true;
#else
	
	DIR* dir;					// pointer to the scanned directory.
	struct dirent* entry;		// pointer to one directory entry.
	struct stat dir_stat;       // used by stat().
	char tmppath[MAX_PATH+1];
	char path[MAX_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL) strcpy(path,p);

	// open the directory for reading
	dir = opendir( checkPath(path) );
	if(!dir) {
		fprintf(stderr, "Cannot read directory '%s'\n", path);
		return;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while ((entry = readdir(dir))) {
		// skip the "." and ".." entries.
		if(strcmp(entry->d_name, ".") == 0)
			continue;
		if(strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(tmppath, sizeof(tmppath), "%s%c%s",path, PATHSEP, entry->d_name);

		// check if the pattern matchs.
		if(entry->d_name && match_wildcard<char>(pattern, entry->d_name)) {
			func( tmppath );
		}
		// check if it is a directory.
		if(stat(tmppath, &dir_stat) == -1) {
			fprintf(stderr, "stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if(S_ISDIR(dir_stat.st_mode)) {
			// decent recursivly
			findfile(tmppath, pat, func);
		}
	}//end while
	closedir(dir);
	return true;
#endif
}
*/




///////////////////////////////////////////////////////////////////////////////
// true when given name exists
bool is_present(const char*name)
{
#ifdef WIN32
	struct stat dir_stat;       // used by stat().
	return ( 0==access(name, F_OK) && stat(name, &dir_stat) != -1 );
#else
	struct stat dir_stat;       // used by stat().
	return ( 0==access(name, F_OK) && stat(name, &dir_stat) != -1 );
#endif
}

///////////////////////////////////////////////////////////////////////////////
// true when given path is a directory
bool is_folder(const char*name)
{
#ifdef WIN32
	struct stat dir_stat;       // used by stat().
	if( 0!=access(name, F_OK) || stat(name, &dir_stat) == -1 )
		return false;
	return ( 0 != (dir_stat.st_mode & _S_IFDIR) );
#else
	struct stat dir_stat;       // used by stat().
	if(0!=access(name, F_OK) || stat(name, &dir_stat) == -1)
		return false;
	return (S_ISDIR(dir_stat.st_mode));
#endif
}
///////////////////////////////////////////////////////////////////////////////
// true when given path is a file
bool is_file(const char*name)
{
#ifdef WIN32
	struct stat dir_stat;       // used by stat().
	if( 0!=access(name, F_OK) || stat(name, &dir_stat) == -1 )
		return false;
	return ( 0 != (dir_stat.st_mode & _S_IFREG) );
#else
	struct stat dir_stat;       // used by stat().
	if(0!=access(name, F_OK) || stat(name, &dir_stat) == -1)
		return false;
	return (S_ISREG(dir_stat.st_mode));
#endif
}

bool file_exists(const char*name)
{
	return is_file(name);
}

bool file_readable (const char*name)
{	// a file is readable if it exists and can be read
	return ( file_exists(name) && access(name,R_OK)==0 );
}

bool file_writable (const char*name)
{
	// a file is writable if it exists as a file and is writable or 
	// if it doesn't exist but could be created and would be writable
	if(is_present(name))
	{
		if(!is_file(name)) return false;
		return access(name,W_OK)==0;
	}
	string<> dir = folder_part(name);
	if( dir.is_empty() ) dir = ".";
	return folder_writable(dir);
}

unsigned long filesize(const char*filename)
{
#ifdef _WIN32
	struct _stat st;
	if( !filename || !_stat(filename, &st) == 0 ) return 0;
#else
	struct  stat st;
	if( !filename ||  !stat(filename, &st) == 0 ) return 0;
#endif
	return st.st_size;
}

bool file_delete (const char* filespec)
{
	return ( is_file(filespec) && remove(filespec)==0 );
}

bool file_rename (const char* old_filespec, const char* new_filespec)
{
	return ( is_file(old_filespec) && rename(old_filespec, new_filespec)==0);
}

bool file_copy (const char* old_filespec, const char* new_filespec)
{
	if(!is_file(old_filespec))
		return false;
	// do an exact copy - to do this, use binary mode
	bool result = true;
	FILE* old_file = fopen(old_filespec,"rb");
	FILE* new_file = fopen(new_filespec,"wb");
	if(!old_file)
		result = false;
	else if(!new_file)
		result = false;
	else
	{
		int byte;
		for ( byte = getc(old_file); byte != EOF; byte = getc(old_file))
			putc(byte,new_file);
	}
	if(old_file) fclose(old_file);
	if(new_file) fclose(new_file);
	return result;
}

bool file_move (const char* old_filespec, const char* new_filespec)
{
	// try to move the file by renaming - if that fails then do a copy and delete the original
	if(file_rename(old_filespec, new_filespec))
		return true;
	if(!file_copy(old_filespec, new_filespec))
		return false;
	// I'm not sure what to do if the delete fails - is that an error?
	// I've made it an error and then delete the copy so that the original state is recovered
	if(file_delete(old_filespec))
		return true;
	file_delete(new_filespec);
	return false;
}













#ifdef _WIN32
static const char* separator_set = "\\/";
static const char preferred_separator = '\\';
#else
static const char* separator_set = "/";
static const char preferred_separator = '/';
#endif

static bool is_separator (char ch)
{
	for (int i = 0; separator_set[i]; ++i)
	{
		if(separator_set[i] == ch)
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// implement string comparison of paths - Unix is case-sensitive, Windoze is case-insensitive

static bool path_compare(const string<>& l, const string<>& r)
{
#ifdef _WIN32
	return lowercase(l) == lowercase(r);
#else
	return l == r;
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// Internal data structure.
/// used to hold the different parts of a filespec
class file_specification
{
private:
	bool m_relative;			// true = relative, false = absolute
	string<> m_drive;			// drive - drive letter (e.g. "c:") or the path for an UNC (e.g. "\\somewhere")
								// empty if not known or on Unix
	vector< string<> > m_path;	// the subdirectory path to follow from the drive
	string<> m_filename;		// the filename
public:
	file_specification(void) : m_relative(false) {}
	~file_specification(void) {}

	bool initialise_folder(const string<>& spec);
	bool initialise_file(const string<>& spec);
	bool simplify(void);
	bool make_absolute(const string<>& root = folder_current_full());
	bool make_absolute(const file_specification& root);
	bool make_relative(const string<>& root);
	bool make_relative(const file_specification& root);
	bool relative(void) const {return m_relative;}
	bool absolute(void) const {return !relative();}
	void set_relative(void) {m_relative = true;}
	void set_absolute(void) {m_relative = false;}

	const string<>& drive(void) const {return m_drive;}
	string<>& drive(void) {return m_drive;}
	void set_drive(const string<>& drive) {m_drive = drive;}

	const vector< string<> >& path(void) const {return m_path;}
	vector< string<> >& path(void) {return m_path;}
	void set_path(const vector< string<> >& path) {m_path = path;}

	void add_subpath(const string<>& subpath) {m_path.push(subpath);}
	size_t subpath_size(void) const {return m_path.size();}
	const string<>& subpath_element(size_t i) const {return m_path[i];}
	void subpath_erase(size_t i) {m_path.removeindex(i);}

	const string<>& file(void) const {return m_filename;}
	string<>& file(void) {return m_filename;}
	void set_file(const string<>& file) {m_filename = file;}

	string<> image(void) const;
};

bool file_specification::initialise_folder(const string<>& folder_spec)
{
	string<> spec = folder_spec;
	m_relative = true;
	m_drive.clear();
	m_path.clear();
	size_t i = 0;
#if defined(WIN32) || defined(CYGWIN)
	// first split off the drive letter or UNC prefix on Windows - the Cygwin environment supports these too
	if(spec.size() >= 2 && isalpha(spec[0]) && spec[1] == ':')
	{
		// found a drive letter
		i = 2;
		m_drive = spec.substr(0, 2);
		m_relative = false;
		// if there is a drive but no path or a relative path, get the current path for this drive and prepend it to the path
		if(i == spec.size() || !is_separator(spec[i]))
		{
#ifdef WIN32
			// getdcwd requires the drive number (1..26) not the letter (A..Z)
			char path [MAX_PATH+1];
			int drivenum = upcase<char>(m_drive[0]) - 'A' + 1;
			if(_getdcwd(drivenum, path, MAX_PATH+1))
			{
				// the path includes the drive so we have the drive info twice
				// need to prepend this absolute path to the spec such that any remaining relative path is still retained
				if(!is_separator(path[hstrlen(path)-1]))
					spec.insert(2, 1, preferred_separator);
				spec.insert(2, path+2);
			}
			else
#endif
			{
			// non-existent drive - fill in just the root directory
			spec.insert(2, 1, preferred_separator);
			}
		}
	}
	else if(spec.size() >= 2 && is_separator(spec[0]) && is_separator(spec[1]))
	{
		// found an UNC prefix
		i = 2;
		// find the end of the prefix by scanning for the next seperator or the end of the spec
		while (i < spec.size() && !is_separator(spec[i])) i++;
		m_drive = spec.substr(0, i);
		m_relative = false;
	}
#endif
	// check whether the path is absolute or relative and discard the leading / if absolute
	if(i < spec.size() && is_separator(spec[i]))
	{
		m_relative = false;
		i++;
#ifdef _WIN32
		// if there's no drive, fill it in on Windows since absolute paths must have a drive
		if( m_drive.is_empty())
		{
			m_drive += (char)(_getdrive() - 1 + 'A');
			m_drive += ':';
		}
#endif
	}
	// now extract the path elements - note that a trailing / is not significant since /a/b/c/ === /a/b/c
	// also note that the leading / has been discarded - all paths are relative
	// if absolute() is set, then paths are relative to the drive, else they are relative to the current path
	size_t start = i;
	while(i <= spec.size())
	{
		if(i == spec.size())
		{
			// path element terminated by the end of the string
			// discard this element if it is zero length because that represents the trailing /
			if(i != start)
				m_path.push(spec.substr(start, i-start));
		}
		else if(is_separator(spec[i]))
		{
			// path element terminated by a separator
			m_path.push(spec.substr(start, i-start));
			start = i+1;
		}
		i++;
	}
	// TODO - some error handling?
	return true;
}

bool file_specification::initialise_file(const string<>& spec)
{
  m_filename.clear();
  // remove last element as the file and then treat the rest as a folder
  size_t i = spec.size();
  while (--i)
  {
    if(is_separator(spec[i]))
      break;
#ifdef _WIN32
    // on windoze you can say a:fred.txt so the colon separates the path from the filename
    else if(i == 1 && spec[i] == ':')
      break;
#endif
  }
  m_filename = spec.substr(i+1,spec.size()-i-1);
  // TODO - some error handling?
  return initialise_folder(spec.substr(0,i+1));
}

bool file_specification::simplify(void)
{
  // simplify the path by removing unnecessary . and .. entries - Note that zero-length entries are treated like .
  for (size_t i = 0; i < m_path.size(); )
  {
    if( m_path[i].is_empty() || m_path[i]==(".") )
    {
      // found . or null
      // these both mean do nothing - so simply delete this element
      m_path.removeindex(i);
    }
    else if(m_path[i] == "..")
    {
      // found ..
      if(i == 0 && !m_relative)
      {
        // up from the root does nothing so can be deleted
        m_path.removeindex(i);
        i++;
      }
      else if(i == 0 || m_path[i-1]==("..") )
      {
        // the first element of a relative path or the previous element is .. then keep it
        i++;
      }
      else
      {
        // otherwise delete this element and the previous one
        m_path.removeindex(i-1,2);
        i--;
      }
    }
    // keep all other elements
    else
      i++;
  }
  // TODO - error checking?
  return true;
}

bool file_specification::make_absolute(const string<>& root)
{
  if(absolute()) return true;
  file_specification rootspec;
  rootspec.initialise_folder(root);
  return make_absolute(rootspec);
}

bool file_specification::make_absolute(const file_specification& rootspec)
{
	if( this->absolute() )
		return true;
	// now append this's relative path and filename to the root's absolute path
	file_specification result = rootspec;
	for (size_t i = 0; i < subpath_size(); ++i)
		result.add_subpath(subpath_element(i));
	result.set_file(file());
	// now the rootspec is the absolute path, so transfer it to this
	*this = result;
	// and simplify to get rid of any unwanted .. or . elements
	simplify();
	return true;
}

bool file_specification::make_relative(const string<>& root)
{
  if(relative()) return true;
  file_specification rootspec;
  rootspec.initialise_folder(root);
  return make_relative(rootspec);
}

bool file_specification::make_relative(const file_specification& rootspec)
{
	if( this->relative() )
		return true;

	// now compare elements of the root with elements of this to find the common path
	// if the drives are different, no conversion can take place, else clear the drive
	if( !path_compare(drive(), rootspec.drive()) )
		return true;
	set_drive("");
	// first remove leading elements that are identical to the corresponding element in root
	size_t i = 0;
	while(subpath_size() > 0 && i < rootspec.subpath_size() && path_compare(subpath_element(0), rootspec.subpath_element(i)))
	{
		subpath_erase(0);
		i++;
	}
	// now add a .. prefix for every element in root that is different from this
	while (i < rootspec.subpath_size())
	{
		m_path.insert("..", 1, 0);
		i++;
	}
	set_relative();
	return true;
}

string<> file_specification::image(void) const
{
	string<> result = m_drive;
	if( absolute() )
		result += preferred_separator;
	if( !m_path.is_empty() )
	{
		vector< string<> >::iterator iter(m_path);
		if( iter )
			result << *iter++;
		while( iter )
			result << preferred_separator << *iter++;
	}
	else if( relative() )
		result << '.';
	if( !m_filename.is_empty() )
	{
		result << preferred_separator;
		result << m_filename;
	}
	return result;
}




const int read_mode = 0444;
const int write_mode = 0222;
const int execute_mode = 0111;
const int none_mode = 0000;
const int read_write_mode = read_mode | write_mode;
const int all_mode = read_mode | write_mode | execute_mode;
const int owner_mask = 0700;
const int group_mask = 0070;
const int other_mask = 0007;
const int non_owner_mask = group_mask | other_mask;
const int all_mask = owner_mask | group_mask | other_mask;
const int read_mode_all = read_mode & all_mask;
const int read_write_mode_owner_read_mode_all = (read_write_mode & owner_mask) | (read_mode & non_owner_mask);
const int read_mode_owner_only = read_mode & owner_mask;
const int read_write_mode_owner_only = read_write_mode & owner_mask;

bool file_set_mode (const char* filespec, int mode)
{
	return (is_file(filespec) && chmod(filespec, mode)==0);
}

time_t file_created (const char* filespec)
{
	struct stat buf;
	if(!(stat(filespec, &buf) == 0)) return 0;
	return buf.st_ctime;
}

time_t file_modified (const char* filespec)
{
	struct stat buf;
	if(!(stat(filespec, &buf) == 0)) return 0;
	return buf.st_mtime;
}

time_t file_accessed (const char* filespec)
{
	struct stat buf;
	if(!(stat(filespec, &buf) == 0)) return 0;
	return buf.st_atime;
}

string<> create_filespec (const char* directory, const char* filename)
{
	string<> result = directory;
	// if directory is empty then no directory part will be added
	// add trailing slash if the directory was specified and does not have a trailing slash
	if(!result.is_empty() && !is_separator(result[result.size()-1]))
		result << preferred_separator;
	// if filename is null or empty, nothing will be added so the path is then a directory path
	result += filename;
	return result;
}

string<> create_filespec (const char* directory, const char* basename, const char* extension)
{
	return create_filespec(directory, create_filename(basename, extension));
}

string<> create_filename(const char* basename, const char* extension)
{
	string<> name = basename;
	// extension is optional - so the dot is also optional
	if(extension && *extension)
	{
		if(extension[0] != '.')
			name << '.';
		name << extension;
	}
	return name;
}

////////////////////////////////////////////////////////////////////////////////
// folder functions

bool folder_create (const char* directory)
{
#ifdef _WIN32
	return mkdir(directory) == 0;
#else
	return mkdir(directory, 0777) == 0;
#endif
}

bool folder_exists (const char* directory)
{
	return is_folder(directory);
}

bool folder_readable (const char* directory)
{
	// a folder is readable if it exists and has read access
	string<> dir = directory;
	if( dir.is_empty() )
		dir = ".";
	if( !folder_exists(dir) )
		return false;
	return access(dir.c_str(),R_OK)==0;
}

bool folder_writable (const char* directory)
{
	// a folder is writable if it exists and has write access
	string<> dir = directory;
	if( dir.is_empty() )
		dir = ".";
	if( !folder_exists(dir) )
		return false;
	return access(dir.c_str(),W_OK)==0;
}

bool folder_delete(const char* directory, bool recurse)
{
	string<> dir = directory;
	if( dir.is_empty() )
		dir = ".";
	if( !folder_exists(dir) )
		return false;
	
	bool result = true;
	// depth-first traversal ensures that directory contents are deleted 
	// before trying to delete the directory itself
	if(recurse)
	{
		vector< string<> > ret = folder_subdirectories(dir);
		vector< string<> >::iterator iter(ret);
		while( iter )
			result &= folder_delete(folder_down(dir,*iter++),true);
		
		ret = folder_files(dir);
		iter = ret;
		while( iter )
	      result &= file_delete(create_filespec(dir, *iter++));
	}
	result &= (rmdir(dir.c_str())!=0);
	return result;
}

bool folder_rename (const char* old_directory, const char* new_directory)
{
	if( !old_directory || !new_directory || !folder_exists(old_directory) )
		return false;
	return rename(old_directory, new_directory)==0;
}

bool folder_is_empty(const char* directory)
{
	string<> dir = (!directory || !*directory) ? "." : directory;
	bool result = true;
#ifdef _WIN32
	string<> wildcard = create_filespec(dir, "*.*");
	long handle = -1;
	_finddata_t fileinfo;
	bool OK;
	for ( OK=(handle = _findfirst((char*)wildcard.c_str(), &fileinfo)) != -1; OK; OK = (_findnext(handle, &fileinfo)==0))
	{
		if(strcmp(fileinfo.name,".")!=0 && strcmp(fileinfo.name,"..")!=0)
		{
			result = false;
			break;
		}
	}
	_findclose(handle);
#else
	DIR* d = opendir(dir.c_str());
	if(d)
	{
		for (dirent* entry = readdir(d); entry; entry = readdir(d))
		{
			if( strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0)
			{
				result = false;
				break;
			}
		}
		closedir(d);
	}
#endif
	return result;
}

bool folder_set_current(const string<>& folder)
{
  if(!folder_exists(folder))
    return false;
#ifdef _WIN32
  // Windose implementation - this returns non-zero for success
  return (SetCurrentDirectory(folder.c_str()) != 0);
#else
  // Unix implementation - this returns zero for success
  return (chdir(folder.c_str()) == 0);
#endif
}

string<> folder_current (void)
{
  return ".";
}

string<> folder_current_full(void)
{
  // It's not clear from the documentation whether the buffer for a path should be one byte longer
  // than the maximum path length to allow for the null termination, so I have made it so anyway
#ifdef _WIN32
  char abspath [MAX_PATH+1];
  return string<>(_fullpath(abspath, ".", MAX_PATH+1));
#else
  char pathname [MAXPATHLEN+1];
  getcwd(pathname,MAXPATHLEN+1);
  return string<>(pathname);
#endif
}

string<> folder_down (const char* directory, const char* subdirectory)
{
	file_specification spec;
	spec.initialise_folder(directory);
	spec.add_subpath(subdirectory);
	return spec.image();
}

string<> folder_up (const char* directory, size_t levels)
{
	file_specification spec;
	spec.initialise_folder(directory);
	for (size_t i = 0; i < levels; ++i)
		spec.add_subpath("..");
	spec.simplify();
	return spec.image();
}

vector< string<> > folder_subdirectories (const char* directory)
{
	return folder_wildcard(directory, "*", true, false);
}

vector< string<> > folder_files (const char* directory)
{
	return folder_wildcard(directory, "*", false, true);
}

vector< string<> > folder_all(const char* directory)
{
	return folder_wildcard(directory, "*", true, true);
}

vector< string<> > folder_wildcard (const char* directory, const char* wild, bool subdirs, bool files)
{
	const char* dir = (!directory || !*directory) ? "." : directory;
	vector< string<> > results;
#ifdef _WIN32
	string<> wildcard = create_filespec(dir, wild);
	long handle = -1;
	_finddata_t fileinfo;
	bool OK;
	
	for (OK = (handle = _findfirst((char*)wildcard.c_str(), &fileinfo)) != -1; OK; OK = (_findnext(handle, &fileinfo)==0))
	{
		if(strcmp(fileinfo.name,".")!=0 && strcmp(fileinfo.name,"..")!=0)
			if((subdirs && (fileinfo.attrib & _A_SUBDIR)) || (files && !(fileinfo.attrib & _A_SUBDIR)))
				results.push(fileinfo.name);
	}
	_findclose(handle);
#else
	DIR* d = opendir(dir);
	if(d)
	{
		dirent* entry;
		for (entry = readdir(d); entry; entry = readdir(d))
		{
			if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0)
			{
				string<> subpath = create_filespec(dir, entry->d_name);
				if(((subdirs && is_folder(subpath)) || (files && is_file(subpath))) && (match_wildcard<char>(wild, entry->d_name)))
					results.push(entry->d_name);
			}
		}
		closedir(d);
	}
#endif
	return results;
}

string<> folder_home (void)
{
	if( getenv("HOME") )
		return string<>(getenv("HOME"));
#ifdef _WIN32
	if( getenv("HOMEDRIVE") || getenv("HOMEPATH") )
		return string<>(getenv("HOMEDRIVE")) + string<>(getenv("HOMEPATH"));
	return "C:\\";
#else
	if( getenv("USER") )
		return folder_down("/home", string<>(getenv("USER")));
	if(getenv("USERNAME"))
		return folder_down("/home", string<>(getenv("USERNAME")));
	return "";
#endif
}

////////////////////////////////////////////////////////////////////////////////
// path functions convert between full and relative paths

bool is_full_path(const string<>& path)
{
	file_specification spec;
	spec.initialise_folder(path.is_empty() ? string<>(".") : path);
	return spec.absolute();
}

bool is_relative_path(const string<>& path)
{
	file_specification spec;
	spec.initialise_folder(path.is_empty() ? string<>(".") : path);
	return spec.relative();
}

static string<> full_path(const string<>& root, const string<>& path)
{
	// convert path to a full path using root as the start point for relative paths
	// decompose the path and test whether it is already an absolute path, in which case just return it
	file_specification spec;
	spec.initialise_folder(path.is_empty() ? string<>(".") : path);
	if(spec.absolute()) return spec.image();
	// okay, so the path is relative after all, so we need to combine it with the root path
	// decompose the root path and check whether it is relative
	file_specification rootspec;
	rootspec.initialise_folder(root.is_empty() ? string<>(".") : root);
	if(rootspec.relative())
		rootspec.make_absolute();
	// Now do the conversion of the path relative to the root
	spec.make_absolute(rootspec);
	return spec.image();
}

static string<> relative_path(const string<>& root, const string<>& path)
{
	// convert path to a relative path, using the root path as its starting point
	// first convert both paths to full paths relative to CWD
	file_specification rootspec;
	rootspec.initialise_folder(root.is_empty() ? string<>(".") : root);
	if(rootspec.relative())
		rootspec.make_absolute();
	file_specification spec;
	spec.initialise_folder(path.is_empty() ? string<>(".") : path);
	if(spec.relative())
		spec.make_absolute();
	// now make path spec relative to the root spec
	spec.make_relative(rootspec);
	return spec.image();
}

string<> folder_to_path (const char* root, const char* folder)
{
	return full_path(root, folder);
}

string<> filespec_to_path (const char* root, const char* filespec)
{
	return create_filespec(folder_to_path(root, folder_part(filespec)),filename_part(filespec));
}

string<> folder_to_path(const char* folder)
{
	return folder_to_path(folder_current(), folder);
}

string<> filespec_to_path(const char* filespec)
{
	return filespec_to_path(folder_current(), filespec);
}

string<> folder_to_relative_path(const string<>& root, const string<>& folder)
{
	return relative_path(root, folder);
}

string<> filespec_to_relative_path(const string<>& root, const string<>& spec)
{
	return create_filespec(folder_to_relative_path(root, folder_part(spec)),filename_part(spec));
}

string<> folder_to_relative_path(const string<>& folder)
{
	return folder_to_relative_path(folder_current(), folder);
}

string<> filespec_to_relative_path(const string<>& filespec)
{
	return filespec_to_relative_path(folder_current(), filespec);
}

string<> folder_append_separator(const string<>& folder)
{
	string<> result = folder;
	if(!is_separator(result[result.size()-1]))
		result += preferred_separator;
	return result;
}

////////////////////////////////////////////////////////////////////////////////

string<> basename_part (const char* spec)
{
	string<> fname;
	if( spec && *spec )
	{
		// scan back through filename until a '.' is found and remove suffix
		// the whole filename is the basename if there is no '.'
		const char* ip = strrchr(spec, '.');
		// observe Unix convention that a dot at the start of a filename is part of the basename, not the extension
		if( ip && ip != spec)
			fname.append(spec, ip-spec);
		else
			fname.append(spec);
	}
	return fname;
}

string<> filename_part (const char* spec)
{
	// scan back through filename until a preferred_separator is found and remove prefix;
	// if there is no preferred_separator then remove nothing, i.e. the whole filespec is filename
	if(spec)
	{
		const char* ip = spec+hstrlen(spec);
		while(ip>spec)
		{
			if( is_separator( *spec ) )
				return string<>(ip+1);
			else
				--ip;
		}
	}
	return spec;
}

string<> extension_part (const char *spec)
{
	if( spec && *spec )
	{
		string<> fname = filename_part(spec);
		// scan back through filename until a '.' is found and remove prefix;
		size_t i = fname.find_last_of('.');
		// observe Unix convention that a dot at the start of a filename is part of the name, not the extension;
		if( i!=0 && i!= fname.npos )
			fname.clear(0, i);
		else
			fname.clear();
		return fname;
	}
	return string<>();
	
}

string<> folder_part (const char * spec)
{
	if(spec && *spec)
	{
		// scan back through filename until a separator is found and remove prefix
		// if there is no separator, remove the whole
		const char *ip = spec+hstrlen(spec);
		while( ip>spec )
		{
			if( is_separator(*ip) )
				return string<>( spec, ip-spec);
			--ip;
		}
	}
	return string<>();
}

vector< string<> > filespec_elements(const char * filespec)
{
	file_specification spec;
	spec.initialise_file(filespec);
	vector< string<> > result = spec.path();
	if(!spec.drive().is_empty()) result.insert(spec.drive(),0);
	if(!spec.file().is_empty()) result.push(spec.file());
	return result;
}

vector< string<> > folder_elements(const char * folder)
{
	file_specification spec;
	spec.initialise_folder(folder);
	vector< string<> > result = spec.path();
	if(!spec.drive().is_empty()) result.insert(spec.drive(),0);
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// mimic the command lookup used by the shell

// Windows looks at the following locations:
// 1) application root
// 2) current directory
// 3) 32-bit system directory
// 4) 16-bit system directory
// 5) windows system directory
// 6) %path%
// currently only (2) and (6) has been implemented although many system folders are on the path anyway
// also implement the implied .exe extension on commands with no path (see CreateProcess documentation)
// TODO - PATHEXT handling to find non-exe executables

string<> path_lookup (const char* command)
{
	string<> path;
	path << '.' << PATH_SPLITTER << getenv("PATH");
	return lookup(command, path);
}

string<> lookup (const char* command, const char* path, const char* splitter)
{
	// first check whether the command is already a path and check whether it exists
	if(!folder_part(command).is_empty())
	{
		if(file_exists(command))
			return command;
	}
	else
	{
		// command is just a name - so do path lookup
		vector< string<> > paths = split<char>(path, splitter);
		for (size_t i = 0; i < paths.size(); ++i)
		{
			string<> spec = create_filespec(paths[i], command);
			if(file_exists(spec))
			{
				return spec;
			}
		}
	}
#ifdef _WIN32
	// if there is no extension, try recursing on each possible extension
	// TODO iterate through PATHEXT
	if(extension_part(command).is_empty())
		return lookup(create_filespec(folder_part(command), basename_part(command), "exe"), path, splitter);
#endif
	// if path lookup failed, return is_empty string to indicate error
	return string<>();
}

////////////////////////////////////////////////////////////////////////////////

string<> install_path(const char* argv0)
{
	string<> bin_directory = folder_part(argv0);
	if(bin_directory.is_empty())
	{
		// do path lookup to find the executable path
		bin_directory = folder_part(path_lookup(argv0));
	}
	return bin_directory;
}

////////////////////////////////////////////////////////////////////////////////

NAMESPACE_END(basics)
