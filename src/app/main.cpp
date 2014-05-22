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
        adapter::plotcirc_save_to_png( pc, (dir_objects + "face.png").c_str() );
                
        cmn::plotcirc_db_pt pcd = std::make_shared<cmn::plotcirc_db_t>();
        alg::plotcirc_db_add( pcd, pc );
        
        //adapter::plotcirc_db_export_to_sqlite( pcd, sqlite_db_path );
        
        
        cmn::plotcirc_db_pt pcd2 = std::make_shared<cmn::plotcirc_db_t>();
        adapter::plotcirc_db_import_from_sqlite( pcd, sqlite_db_path );
        if(!pcd2->plotcircs.empty())
        {
                cmn::plotcirc_pt pc2 = *pcd2->plotcircs.begin();
                int a = 10; ++a;
        }
        
        return 0;
}