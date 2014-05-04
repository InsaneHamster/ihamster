#include <cmn/plotcirc.hpp>
#include <cmn/image.hpp>
//#include <cmn/point.hpp>
#include <cmn/rect.hpp>
#include <math.h>
#include <memory.h>

namespace cmn
{

static void 
trace_pat(      image_t * const img_dst, int img_dst_row,                 //aka pattern
                image_t * const img_src, float const max_img_value,
                point2f_t const center, point2f_t dir )
{
        image_header_t const & hs = img_src->header;
        image_header_t const & hd = img_dst->header;
        
        rect2f_t bounds_src(0,0, hs.width, hs.height);
        
        float t = 0;                
        do
        {
                point2f_t pt = center + dir * t;
                if( bounds_src.is_inside( pt ) )
                {
                        uint16_t value_src = img_src->row<uint16_t>(pt.y)[(int)pt.x];
                        float value_dst = value_src / max_img_value;
                        img_dst->row<float>(img_dst_row)[(int)t] = value_dst;
                }
                else
                        break;  //we are out of source                
                ++t;
        } while( t < hd.width );        
}
        

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
        
        
//image in format g16, as in alg/pattern_sub. max_img_value - maximum brightness in img present [0...65535]
plotcirc_pat_pt     
plotcirc_pattern_create( image_pt const & img, point2f_t weight_center, int max_img_value )
{
        image_header_t const & hs = img->header;                
        int width = find_max_distance( hs, weight_center ); //hs.width * hs.width + hs.height * hs.height;
        
        plotcirc_pat_pt pp = std::make_shared<plotcirc_pat_t>();
        pp->img = image_create( width, plotcirc_discr, pitch_default, format_gf );        
        memset( pp->img->bytes, 0, pp->img->header.pitch * pp->img->header.height );
                
        float angle_step = 2 * M_PI / plotcirc_discr;        
        image_t * img_src = img.get();
        image_t * img_dst = pp->img.get();
        
        
        for( int i = 0; i < plotcirc_discr; ++i )        
        {
                float angle = i * angle_step;
                point2f_t dir( cos(angle), sin(angle) );
                trace_pat( img_dst, i, img_src, max_img_value, weight_center, dir );
        }
                
        return pp;
}

static void 
trace_test(     image_t * const img_dst, int img_dst_row,
                image_t * const img_src,
                point2f_t const center, point2f_t dir )
{
        image_header_t const & hs = img_src->header;
        image_header_t const & hd = img_dst->header;
        
        rect2f_t bounds_src(0,0, hs.width, hs.height);
        
        float t = 0;                
        do
        {
                point2f_t pt = center + dir * t;
                if( bounds_src.is_inside( pt ) )
                {
                        bool value_src = image_bw_readpixel( img_src, pt.x, pt.y );
                        image_bw_writepixel( img_dst, t, img_dst_row, value_src );                        
                }
                else
                        break;  //we are out of source                
                ++t;
        } while( t < hd.width );        
}

plotcirc_test_pt    
plotcirc_test_create( image_pt const & img, point2f_t weight_center )
{
        image_header_t const & hs = img->header;                
        int width = find_max_distance( hs, weight_center ); //hs.width * hs.width + hs.height * hs.height;
        
        plotcirc_test_pt pt = std::make_shared<plotcirc_test_t>();
        pt->img = image_create( width, plotcirc_discr, pitch_default, format_bw );
        memset( pt->img->bytes, 0, pt->img->header.pitch * pt->img->header.height );
        
        float angle_step = 2 * M_PI / plotcirc_discr;        
        image_t * img_src = img.get();
        image_t * img_dst = pt->img.get();
        
        for( int i = 0; i < plotcirc_discr; ++i )        
        {
                float angle = i * angle_step;
                point2f_t dir( cos(angle), sin(angle) );
                trace_test( img_dst, i, img_src, weight_center, dir );
        }
                
        return pt;
}

        
}       //end of cmn