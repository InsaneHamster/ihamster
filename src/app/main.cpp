#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <alg/plotcirc_db.hpp>
#include <cmn/plotcirc.hpp>

#include <adapter/image.hpp>
#include <adapter/plotcirc.hpp>
#include <adapter/filesystem.hpp>
#include <adapter/config.hpp>

#include <stdio.h>
#include <string>
#include <regex>


static void handle_source_png( std::string const & dir_dst, int file_index, std::string const & file )
{
        char buf[256];        
        cmn::image_pt img = adapter::image_create_from_png( file.c_str() );
        
        std::vector< alg::watershed_object_t > wov;        
        alg::watershed( &wov, 0, img );
        
        auto order_by_square = [](alg::watershed_object_t const & wl, alg:: watershed_object_t const & wr ) -> bool
        {
                return wl.square > wr.square;
        };
        std::sort(wov.begin(), wov.end(), order_by_square);
                
        int obj_size = std::min( (int)wov.size(), 4 );           //we assume face will be in 4 first...        
        for( int i = 0; i <  obj_size; ++i )
        {                
                sprintf( buf, "%03d_%d", file_index, i );        
                alg::watershed_object_t const & wo = wov[i];
                alg::watershed_object_save_to_png( &wo, (dir_dst + PATH_DELIMITER_C + buf).c_str() );
        }        
}

static void find_objects( std::string const & dir_dst, std::string const & dir_src )
{
        using namespace adapter;
        std::vector<fs_file_info_t> filesv;        
                        
        fs_dir_contents( &filesv, dir_src );
        std::regex rx(".*\\.png");        
        
        int filesv_size = filesv.size();
        for( int i = 0; i != filesv_size; ++i )
        {
                fs_file_info_t const & fi = filesv[i];
                std::match_results<std::string::const_iterator> mr;
                if( fi.type == fs_file_type_file && std::regex_match( fi.name, mr, rx ) )
                {
                        //it's a png file!
                        try
                        {
                                 handle_source_png( dir_dst, i, dir_src + PATH_DELIMITER_C + fi.name );                        
                        }
                        catch( ... )
                        {
                        }
                }
        }
}

static void write_objects( std::string const & seq )
{
        
}

int main( int argc, char const * argv[] )
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/faces_png";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects";

        find_objects( dir_objects, dir_src );
}


#if 0
int main()
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects/";
        std::string sqlite_db_path = adapter::config_sqlite_db_path();
        
        adapter::fs_make_dir( dir_objects );
                
        cmn::image_pt img = adapter::image_create_from_png( (dir_src + "c.png").c_str() );
        
        std::vector< alg::watershed_object_t > wo;
        cmn::image_pt img_watershed;
        alg::watershed( &wo, &img_watershed, img );
        
        adapter::image_save_to_png( img_watershed, (dir_dst + "a_colored.png").c_str() );
        printf("found %d objects\n", (int)wo.size());
    
        alg::watershed_objects_save_to_png( wo, dir_objects );
        
        cmn::plotcirc_pt pc = cmn::plotcirc_create( wo[3].img, wo[3].wc );        
        pc->name = cmn::name_create("face");
        pc->name_sub = 0;
        adapter::plotcirc_save_to_png( pc, (dir_objects + "face.png").c_str() );
                
        cmn::plotcirc_db_pt pcd = std::make_shared<cmn::plotcirc_db_t>();
        alg::plotcirc_db_add( pcd, pc );
        
        //alg::plotcirc_db_export_to_sqlite( pcd, sqlite_db_path );
        
        return 0;
}
#endif