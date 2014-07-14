#pragma once
#include <cmn/fwd.hpp>
#include <cmn/point.hpp>
#include <alg/seg_object.hpp>


namespace alg
{

//on input image in format rgba8
//on output: binary masks of found images. ready to pass to pattern matching
//on ouput optionally: colored image to output for debug or other purposes. 
void watershed( std::vector< seg_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img );

//img_lab - f32 CIE L*a*b* format
void watershed2( std::vector< seg_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img_lab );

void watershed_test();
void watershed2_test();

}