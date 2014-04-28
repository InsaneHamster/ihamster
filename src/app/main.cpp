#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <adapter/image.hpp>
#include <stdio.h>


int main()
{
        char const * path = "/home/tot/c.png";
        cmn::image_pt img = adapter::image_create_from_png( path );
        
        std::vector< alg::watershed_object_t > wo;
        cmn::image_pt img_watershed;
        alg::watershed( &wo, &img_watershed, img );
        
        adapter::image_save_to_png( "/home/tot/a_colored.png", img_watershed );
        printf("found %d objects\n", (int)wo.size());
        
        return 0;
}