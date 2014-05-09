#include <cmn/plotcirc.hpp>
#include <cmn/image.hpp>
//#include <cmn/point.hpp>
#include <cmn/rect.hpp>
#include <math.h>
#include <memory.h>
#include <algorithm>

namespace cmn
{

#if 0
static int 
find_max_distance(image_header_t const & hs, point2f_t weight_center)
{
        float lt = weight_center.distance_sq(point2f_t(0,0));
        float lb = weight_center.distance_sq(point2f_t(0, hs.height));
        float rt = weight_center.distance_sq(point2f_t(hs.width,0));
        float rb = weight_center.distance_sq(point2f_t(hs.width,hs.height));
        
        float m1 = std::max( lt, lb );
        float m2 = std::max( rt, rb );
        float d_sq = std::max(m1, m2);
        float d = sqrt( d_sq );
        return (int)ceil( d );
}
#endif
        
static void 
trace(     plotcirc_t * const pt_row, int img_dst_row,
           image_t * const img_src,
           point2f_t const center, point2f_t dir, float const hypotenuse )
{
        image_header_t const & hs = img_src->header;                
        rect2f_t bounds_src(0,0, hs.width, hs.height);        
        float t = 1;
        
        bool    inside_start = image_bw_readpixel( img_src, center.x, center.y );             //inside object that is
        float   t_start = 0;
        
        do
        {
                point2f_t pt = center + dir * t;
                if( bounds_src.is_inside( pt ) )
                {
                        bool const inside_cur = image_bw_readpixel( img_src, pt.x, pt.y );
                        if( inside_cur != inside_start )
                        {       //we are at a border, record this fact
                                if( inside_start )      //we are inside an object!
                                {
                                        pt_row->rows[img_dst_row].push_back( point2f_t( t_start/hypotenuse, t/hypotenuse ) );                                        
                                        inside_start = inside_cur;
                                }
                                else
                                {       //we are outside an object, just go on... but record starting of the inside region
                                        inside_start = inside_cur;
                                        t_start = t;
                                }
                        }                                                
                }
                else
                        break;  //we are out of source
                ++t;
        } while( 1 );
        
        //sort this
        std::sort( pt_row->rows[img_dst_row].begin(), pt_row->rows[img_dst_row].end(), plotcirc_length_sort_t() );
        pt_row->numpoints += (int)pt_row->rows[img_dst_row].size();
}

plotcirc_pt    
plotcirc_create( image_pt const & img, point2f_t weight_center )
{        
        image_header_t const & hs = img->header;
        //int width = find_max_distance( hs, weight_center ); //hs.width * hs.width + hs.height * hs.height;
        
        plotcirc_pt pt = std::make_shared<plotcirc_t>();
        plotcirc_t * pt_row = pt.get();
                        
        float angle_step = 2 * M_PI / plotcirc_discr;
        image_t * img_src = img.get(); 
        float hypotenuse = sqrt(hs.width * hs.width + hs.height * hs.height);
        
        for( int i = 0; i < plotcirc_discr; ++i )        
        {
                float angle = i * angle_step;
                point2f_t dir( cos(angle), sin(angle) );
                trace( pt_row, i, img_src, weight_center, dir, hypotenuse );
        }
                
        return pt;
}
    
}       //end of cmn