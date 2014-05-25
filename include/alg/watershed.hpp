#pragma once
#include <cmn/fwd.hpp>
#include <cmn/point.hpp>
#include <vector>

namespace alg
{

//bounding box around found object with its contour
struct watershed_object_t
{
        cmn::point2i_t  lt;                      //left-top position in original image
        cmn::image_pt   img;                     //bitmask in format cmn::format_bw . White means presence of an object       
        cmn::point2f_t  wc;                     //wc is weight center - center of mass x,y in img. not a watercloset as you can think!
        int32_t         square;                 //number of pixels in the object
        //cmn::point2f_t  dir;             //direction of longest axis in the object. source is in x_wc, y_wc (see Linear Least Squares method) 
};

//on input image in format rgba8
//on output: binary masks of found images. ready to pass to pattern matching
//on ouput optionally: colored image to output for debug or other purposes. 
void watershed( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img );
void watershed2( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img );


void watershed_object_save_to_png( watershed_object_t const * wo, char const * szPath ); //to png file
void watershed_objects_save_to_png( std::vector< watershed_object_t > const & objects, std::string const & folder );


        
}