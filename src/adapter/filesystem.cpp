#include <adapter/filesystem.hpp>
#include <cmn/log.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <stdlib.h>


namespace adapter
{

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
std::string fs_executable_name()
{
        static_assert(false, "Vova, implement me via _NSGetExecutablePath() !");        //see http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
        return std::string();
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
void fs_dir_contents( std::vector<file_info_t> * files, std::string const & folder )
{
        //DIR d = opendir();
}


} //end of namespace adapter