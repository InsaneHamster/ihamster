#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <alg/plotcirc_db.hpp>
#include <alg/sobel.hpp>

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
        
        std::vector< alg::seg_object_t > wov;        
        alg::watershed( &wov, 0, img );
        
        auto order_by_square = [](alg::seg_object_t const & wl, alg:: seg_object_t const & wr ) -> bool
        {
                return wl.square > wr.square;
        };
        std::sort(wov.begin(), wov.end(), order_by_square);
                
        int obj_size = std::min( (int)wov.size(), 4 );           //we assume face will be in 4 first...        
        for( int i = 0; i <  obj_size; ++i )
        {                
                sprintf( buf, "%03d_%d", file_index, i );        
                alg::seg_object_t const & wo = wov[i];
                alg::seg_object_save_to_png( &wo, (dir_dst + PATH_DELIMITER_C + buf).c_str() );
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

#if 0
int main( int argc, char const * argv[] )
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/faces_png";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects";

        find_objects( dir_objects, dir_src );
}
#endif


#if 1
int main()
{
        //alg::watershed_test();
        alg::sobel_test();
       
        
        return 0;
}
#endif