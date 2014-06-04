#pragma once
#include <cmn/point.hpp>
#include <cmn/fwd.hpp>
#include <vector>

namespace alg
{
        //bounding box around found object with its contour
        struct seg_object_t
        {
                cmn::point2i_t  lt;                      //left-top position in original image
                cmn::image_pt   img;                     //bitmask in format cmn::format_bw . White means presence of an object       
                cmn::point2f_t  wc;                     //wc is weight center - center of mass x,y in img. not a watercloset as you can think!
                int32_t         square;                 //number of pixels in the object
                //cmn::point2f_t  dir;             //direction of longest axis in the object. source is in x_wc, y_wc (see Linear Least Squares method) 
        };
        
        void seg_object_save_to_png( seg_object_t const * wo, char const * szPath );
        void seg_objects_save_to_png( std::vector< seg_object_t > const & objects, std::string const & folder );
        
        //on input g16 image, each index/color - an object. max_color - maximum number in img_quantized
        void seg_create_objects( std::vector< seg_object_t > * objects, uint16_t max_color, cmn::image_pt img_quantized );

        //on output - RGBA image with acid colors
        cmn::image_pt seg_color( cmn::image_pt img_quantized );

        
        
}