#include <adapter/config.hpp>
#include <adapter/filesystem.hpp>

namespace adapter 
{
        std::string config_sqlite_db_path()
        {
                std::string prefs_dir = fs_prefs_dir();
                return prefs_dir + "ihamster.db";
        }
        
}