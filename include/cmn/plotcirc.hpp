#pragma once
#include "fwd.hpp"
#include <cmn/point.hpp>
#include <cmn/name.hpp>
#include <vector>
#include <map>
#include <set>

//abbreviation of a circulat plot
namespace cmn
{

enum { plotcirc_discr = 32 };         //discretization of circular plot, each point will be taken each 360/plotcirc_discr_e degrees
//enum { plotcirc_pat_ };


//it's array of segments sorted from most long to most short. (segement set as begin/end)
//segements are normalized - that is maximum size = 1, normalization is made by hypotenuse
struct plotcirc_t
{
        name_t                   name = name_default();
        point2i_t                img_size;                      //original
        std::vector< point2f_t > rows[plotcirc_discr];
        int                      num_segments = 0;
        
};

struct plotcirc_length_sort_t
{
        bool operator()( point2f_t p1, point2f_t p2 ) const { return (p1.y-p1.x) > (p2.y-p2.x); }
};



struct plotcirc_cmp_t
{
        float diff;            //0...1         0 - identical, 1 - fully different
        
        //in radians. object relative to pattern ( if pattern is '|' and object is '/' then angle == 45degrees, or M_PI/4 radians)
        //TODO: use Linear Least Squares method to determine, refer to http://en.wikipedia.org/wiki/Linear_least_squares_%28mathematics%29 
        float angle;                 
        
        float scale;                 //scale of object relative to pattern ( >1 if test object is larger than pattern)
};


//image has to be binary and purified
plotcirc_pt    plotcirc_create( image_pt const & img, point2f_t weight_center, name_t name = name_default() );

//see in adapter / plotcirc.hpp
//void           plotcirc_print( plotcirc_pt const & pc, std::string const & path );


//database part

struct plotcirc_find_t
{
        cmn::plotcirc_pt     plotcirc;
        cmn::plotcirc_cmp_t  cmp_res;        
};

typedef std::vector<plotcirc_find_t> plotcirc_find_vt;


typedef std::multimap< float, cmn::plotcirc_pt > plotcirc_db_map_t;
struct plotcirc_db_t
{
        plotcirc_db_map_t        begins[cmn::plotcirc_discr];
        plotcirc_db_map_t        ends[cmn::plotcirc_discr];
        std::set<plotcirc_pt>    plotcircs;
};

//typedef std::shared_ptr<plotcirc_db_t> plotcirc_db_pt; fwd.hpp


}