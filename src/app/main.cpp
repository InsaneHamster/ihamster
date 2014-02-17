#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <adapter/image.hpp>
#include <stdio.h>


int main()
{
        char const * path = "/home/tot/a.png";
        cmn::image_pt img = adapter::image_create_from_png( path );
        
        return 0;
}