#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <cmn/plotcirc.hpp>

#include <adapter/image.hpp>
#include <adapter/plotcirc.hpp>

#include <stdio.h>


int main()
{
        char const * path = "/home/tot/c.png";
        cmn::image_pt img = adapter::image_create_from_png( path );
        
        std::vector< alg::watershed_object_t > wo;
        cmn::image_pt img_watershed;
        alg::watershed( &wo, &img_watershed, img );
        
        adapter::image_save_to_png( img_watershed, "/home/tot/a_colored.png" );
        printf("found %d objects\n", (int)wo.size());
        
        alg::watershed_objects_save_to_png( wo, "/home/tot/objects" );
        
        cmn::plotcirc_pt pc = cmn::plotcirc_create( wo[3].img, wo[3].wc );        
        adapter::plotcirc_save_to_png( pc, "/home/tot/objects/face.png" );
        
        return 0;
}