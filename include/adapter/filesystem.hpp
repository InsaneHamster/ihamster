#pragma once
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

namespace adapter 
{
        #define PATH_DELIMITER_C '/'
        #define PATH_DELIMITER_S "/"
        
        class fs_execption : public std::runtime_error
        {
        public:
                fs_execption( int i_errno, std::string const & msg ) : std::runtime_error(msg), m_errno(i_errno){}                
                int m_errno;
        };
        
        enum delimiter_add_et
        {
                delimiter_dont_add = 0,            //don't add PATH_DELIMITER_C to the end (that is usr/lib  )
                delimiter_add = 1                  //add PATH_DELIMITER_C to the end       (that is usr/lib/ )                
        };
                                
        std::string fs_executable_name();
                 
        std::string fs_executable_dir(delimiter_add_et da = delimiter_add);
        
        //currently ==executable on linux (potenrially /usr/share/ihamser), mac - resource folder in a bundle
        std::string fs_resource_dir(delimiter_add_et da = delimiter_add);

        //do u like lisp ?
        //get last component of a path, that is /usr/lib/libslite3.so will return libslite3.so
        // /usr/lib/libslite3.so/ will return libslite3.so/
        std::string fs_reverse_car( std::string const & path );           
        
        //get first/dir component of a path, that is /usr/lib/libslite3.so will return /usr/lib
        std::string fs_reverse_cdr( std::string const & path , delimiter_add_et da = delimiter_add);           
                
        //won't create subfolders, throws if error
        void fs_make_dir(std::string const & path);
        
        //here database with images/patters will be stored, as well as all debug output
        std::string fs_prefs_dir(delimiter_add_et da = delimiter_add);  //on linux ~/.ihamser
        
        
        enum fs_file_type_e
        {
                fs_file_type_file,
                fs_file_type_dir
        };

        struct fs_file_info_t
        {
                std::string     name;
                fs_file_type_e  type;
        };
        
        //read contents of a directory. non-recursive, all in a time, skips hidden/special files
        //links are stated and resolved to file/directory
        void fs_dir_contents( std::vector<fs_file_info_t> * files, std::string const & dir );
        
}