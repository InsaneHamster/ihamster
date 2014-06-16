#include <adapter/filesystem.hpp>
#include <cmn/log.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <stdlib.h>
#include <stddef.h>


namespace adapter
{

	//motivation: http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
	
#if defined( __linux__ )
static const char gs_self[] = "/proc/self/exe";
std::string fs_executable_name()
{                
        //struct stat s;
        //lstat( gs_self, &s ); //doesn't work properly
        
        char  buf[2048];
        ssize_t written = readlink( gs_self, buf, sizeof(buf));
        return std::string( buf, written );                
}
#elif defined( __APPLE__ )
	#include <mach-o/dyld.h>
std::string fs_executable_name()
{
	uint32_t size = 2048;
	std::vector<char> buf(size, '\0');
	int ret = _NSGetExecutablePath(buf.data(), &size);
	if (ret != 0) {
		cmn::log_and_throw<fs_execption>( ERANGE, "adapter::fs_executable_name buffer is not enough, needed size: %d", size );
	}
	return std::string(buf.data());
}
#endif


std::string fs_executable_dir( delimiter_add_et da )
{
        std::string exec_name = fs_executable_name();
        std::string folder = fs_reverse_cdr(exec_name);
        return folder;        
}

std::string fs_resource_dir(delimiter_add_et da )
{
	// TODO: resource folder in a bundle on mac.
	return fs_executable_dir(da);
}


std::string fs_reverse_car( std::string const & path )
{
        size_t pos = path.empty() ? std::string::npos : 
                path.back() == PATH_DELIMITER_C ? path.size()-2 : std::string::npos;
        size_t found_pos = path.find_last_of( PATH_DELIMITER_C, pos );
        if( found_pos == std::string::npos )
                return path;
        else
                return path.substr( found_pos + 1, std::string::npos );
}
        

std::string fs_reverse_cdr( std::string const & path, delimiter_add_et da )
{
        size_t pos = path.empty() ? std::string::npos : 
        path.back() == PATH_DELIMITER_C ? path.size()-2 : std::string::npos;
        size_t found_pos = path.find_last_of( PATH_DELIMITER_C, pos );
        if( found_pos == std::string::npos )
                return std::string();
        if( found_pos == 0 )
                return std::string(1, '/');             //root
        
        return path.substr( 0, found_pos + (size_t)da );
}

std::pair<std::string, std::string> 
fs_name_ext( std::string const & filename )
{
        std::size_t pos = filename.find_last_of('.');
        if( pos == std::string::npos )        
                return std::pair<std::string, std::string>(filename, std::string());
        else
                return std::make_pair( filename.substr(0, pos), filename.substr( pos+1 ) );
}

void fs_make_dir(std::string const & path)
{
        int ret = mkdir( path.c_str(), 0777 ); 
        if( ret == -1 && errno != EEXIST )
        {
                cmn::log_and_throw<fs_execption>( errno, "adapter::fs_make_folder mkdir failed, errno: %d", errno );
        }
}

std::string fs_prefs_dir(delimiter_add_et da)
{
        char const * home = getenv("HOME");
        if( !home || !*home )
        {       //don't use log and throw here.. obviously we most probably don't know where to log and subsystem can be not initialized
                static char const err[] = "error: adapter::fs_prefs_dir - a user has no home directory ${HOME} is null\n";
                fputs( err, stderr );
                throw fs_execption(errno, err);
        }

        std::string ret = std::string(home) + "/ihamster";
        fs_make_dir(ret);        
        if( da == delimiter_add ) ret += PATH_DELIMITER_C;        
//         std::string ret = home;
//         if( ret.back() ==  PATH_DELIMITER_C && !da && ret.size() > 1 ) ret.resize( ret.size()-1 ); 
//         else if( ret.back() != PATH_DELIMITER_C && da ) ret += PATH_DELIMITER_C;        
         return ret;
}


//**********************************  won't work on windows, but we are not going on to run on it now, right ?
void fs_dir_contents( std::vector<fs_file_info_t> * files, std::string const & dir )
{
        DIR * d = opendir( dir.c_str() );
        if( !d )
                cmn::log_and_throw<fs_execption>( errno, "adapter::fs_dir_contents - can't open dir %s, errno: %d", dir.c_str(), errno );
        
        int dir_df = dirfd(d);
        
        size_t name_max = pathconf(dir.c_str(), _PC_NAME_MAX);
        if (name_max == -1)         /* Limit not defined, or error */
               name_max = 255;         /* Take a guess */               //YS: I suspect it might cause buffer overflow though...
        size_t len = offsetof(struct dirent, d_name) + name_max + 1;
        dirent * entryp = (dirent*)malloc(len);
        dirent * gottendata(0);
        std::string filename; 
        
        do
        {
                int err = readdir_r( d, entryp, &gottendata );
                if( err )
                {
                        free(entryp);
                        closedir(d);
                        cmn::log_and_throw<fs_execption>( err, "adapter::fs_dir_contents - readdir_r returned: %d", err );
                }
        
                if( gottendata )
                {                        
                        fs_file_info_t fi;                        
                        fi.name = gottendata->d_name;
                 
                        bool skip = fi.name[0] == '.';
                        
                        if( !skip )
                        switch( gottendata->d_type )
                        {
                                case DT_UNKNOWN:
                                case DT_LNK:
                                {
                                        struct stat s;
                                        filename = dir + PATH_DELIMITER_C + fi.name;
                                        err = stat( filename.c_str(), &s );
                                        if( err )                                        
                                                skip = true;            //bad file!
                                        else
                                        {
                                                if( S_ISREG(s.st_mode) )
                                                        fi.type = fs_file_type_file;
                                                else if( S_ISDIR(s.st_mode) )
                                                        fi.type = fs_file_type_dir;
                                                else
                                                        skip = true;
                                        }
                                }
                                case DT_REG: fi.type = fs_file_type_file; break;
                                case DT_DIR: fi.type = fs_file_type_dir; break;
                                default: skip = true;                                        
                        }
                        
                        if( !skip )                        
                                files->push_back( fi );
                }
        } while( gottendata );
        
        free(entryp);
        closedir(d);
}


} //end of namespace adapter