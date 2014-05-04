#pragma once
#include "fwd.hpp"
#include <cmn/point.hpp>

//abbreviation of a circulat plot
namespace cmn
{

enum { plotcirc_discr = 32 };         //discretization of circular plot, each point will be taken each 360/plotcirc_discr_e degrees
//enum { plotcirc_pat_ };

struct plotcirc_pat_t
{
        cmn::image_pt img;            //float32 image, width = max dstance from weight_center, height == plotcirc_discr,
        
};

//for the test pattern we use another approach:
//size of image is same as for pattern but it is binary
struct plotcirc_test_t
{
        cmn::image_pt img;   
};

struct plotcirc_cmp_t
{
        float similarity;            //0...1         1 - identical, 0 - fully different
        float angle;                 //in radians. object relative to pattern ( if pattern is '|' and object is '/' then angle == 45degrees, or M_PI/4 radians)
        float scale;                 //scale of object relative to pattern ( >1 if object is larger than pattern)
};

//image in format g16, as in alg/pattern_sub. max_img_value - maximum brightness in img present [0...65535]
//weight_center - either center of mass of objects (better results) or just center of the image
plotcirc_pat_pt     plotcirc_pattern_create( image_pt const & img, point2f_t weight_center, int max_img_value );

//image has to be binary and purified
plotcirc_test_pt    plotcirc_test_create( image_pt const & img, point2f_t weight_center );

plotcirc_cmp_t  plotcirc_compare( plotcirc_pat_pt const & pattern, plotcirc_test_pt const & object );

}