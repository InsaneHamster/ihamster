#pragma once
#include <cmn/plotcirc.hpp>

namespace alg
{          
using cmn::plotcirc_t;
using cmn::plotcirc_pt;
using cmn::plotcirc_cmp_t;
        
//see in adapter        
//void plotcirc_db_import_from_sqlite( std::string const & db_path );
//void plotcirc_db_export_to_sqlite( std::string const & db_path );
        
void plotcirc_db_add( cmn::plotcirc_db_pt const & pcd, cmn::plotcirc_pt const & pc );        
void plotcirc_db_find( cmn::plotcirc_find_vt * res, int find_nth_best, cmn::plotcirc_db_pt const & pcd, cmn::plotcirc_pt const & test  );                


void plotcirc_db_import_from_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path );
void plotcirc_db_export_to_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path );


};