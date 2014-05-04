#pragma once
#include <cmn/fwd.hpp>
#include <vector>

namespace alg
{

//bounding box around found object with its contour
struct watershed_object_t
{
        int           x,y;                      //position in original image
        cmn::image_pt img;                      //bitmask in format cmn::format_bw . White means presence of an object       
        float         x_wc, y_wc;               //wc is weight center - center of mass x,y in img. not a watercloset as you can think!
};

//on input image in format rgba8
//on output: binary masks of found images. ready to pass to pattern matching
//on ouput optionally: colored image to output for debug or other purposes. 
void watershed( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img );

        
}