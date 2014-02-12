#pragma once
//abbreviation of a circulat plot
namespace cmn
{

enum { plotcirc_discr = 32 };         //discretization of circular plot, each point will be taken each 360/plotcirc_discr_e degrees
//enum { plotcirc_pat_ };

struct plotcirc_pat_t
{
        cmn::image_pt img;            //width == plotcirc_discr, height = sqrt(pattern_width * pattern_height)/2+1
        
};

struct plotcirc_test_t
{
};

struct plotcirc_cmp_t
{
        float similarity;            //0...1         1 - identical, 0 - fully different
        float angle;                 //in radians. object relative to pattern ( if pattern is '|' and object is '/' then angle == 45degrees, or M_PI/4 radians)
        float scale;                 //scale of object relative to pattern ( >1 if obbject is larger than pattern)
};

//image in format g16, as in alg/pattern_sub
plotcirc_pat_pt     plotcirc_pattern_create( image_pt const & img );

//image has to be binary and purified
plotcirc_test_pt    plotcirc_test_create( image_pt const & img );

plotcirc_cmp_t  plotcirc_compare( plotcirc_pt const & pattern, plotcirc_pt const & object );

}