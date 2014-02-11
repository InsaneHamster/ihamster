#pragma once
//abbreviation of a circulat plot
namespace cmn
{

enum { plotcirc_discr_e = 32 };         //discretization of circular plot, each point will be taken each 360/plotcirc_discr_e degrees
        
struct plotcirc_t
{
};

struct plotcirc_cmp_t
{
        float similarity;            //0...1         1 - identical, 0 - fully different
        float angle;                 //in radians. object relative to pattern ( if pattern is '|' and object is '/' then angle == 45degrees, or M_PI/4 radians)
        float scale;                 //scale of object relative to pattern ( >1 if obbject is larger than pattern)
};

//image has to be binary and purified
plotcirc_pt     plotcirc_create( image_pt const & img );

plotcirc_cmp_t  plotcirc_compare( plotcirc_pt const & pattern, plotcirc_pt const & object );

}