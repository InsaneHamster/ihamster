#pragma once
#include <cmn/fwd.hpp>
#include <string>

namespace adapter
{
        //to png file
        bool plotcirc_save_to_png( cmn::plotcirc_pt const & pc, char const * const szImgPath );
        
        void plotcirc_db_import_from_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path );
        void plotcirc_db_export_to_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path );

}