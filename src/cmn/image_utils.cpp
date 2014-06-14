#include <cmn/image_utils.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <math.h>
#include <float.h>

cmn::color3f_t max_lab(FLT_MAX,FLT_MAX,FLT_MAX);

namespace cmn
{

image_pt image_rgba_from_g8( image_pt const gray )
{
        image_header_t const & hs = gray->header;
        image_pt img_destp = cmn::image_create( hs.width, hs.height, pitch_default, format_rgba );
        image_t * const img_dest = img_destp.get();
        
        for( int y = 0; y < hs.height; ++y )
        {
                uint8_t const * const row_src = gray->row<uint8_t>(y);
                color4b_t * const row_dst = img_dest->row<color4b_t>(y);
                for( int x = 0; x < hs.width; ++x )
                {
                        uint8_t c = row_src[x];                        
                        row_dst[x] = color4b_t(c,c,c,255);
                }
        }
        
        return img_destp;
}


image_pt image_bw_from_g8( image_pt const & gray, uint8_t const border )
{
        image_t * img_src = gray.get();
        image_header_t const & hs = img_src->header;
        int const width = hs.width;
        int const height = hs.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_bw );
        image_t * img_dst = img_dstp.get();
        
        for( int y = 0; y < height; ++y )
        {
                uint8_t const * const row_src= img_src->row<uint8_t>(y);
                uint8_t * const row_dst = img_dst->row<uint8_t>(y);
                
                for( int x = 0; x < width; ++x )
                {
                        bool value = row_src[x] > border;
                        image_bw_writepixel( row_dst, x, value );
                }
        }
        
        return img_dstp;
}

image_pt image_hsva_from_rgba( image_pt const & img_srcp )
{
        image_t const * const img_src = img_srcp.get();
        int const width = img_src->header.width;
        int const height = img_src->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_hsva );
        image_t * const img_dst = img_dstp.get();
        
        
        for( int y = 0; y < height; ++y )
        {
                color4b_t const * const row_src = img_src->row<color4b_t>(y);
                color4b_t * const row_dst = img_dst->row<color4b_t>(y);
                
                for( int x = 0; x < width; ++x )
                {
                        color4b_t const cs = row_src[x];
                        color4b_t & cd = row_dst[x];
                        int sel;
                        
                        int const maxc1 = cs.r > cs.g ? (sel=0, cs.r) : (sel=1, cs.g);                        
                        int const maxc = maxc1 > cs.b ? maxc1 : (sel=2, cs.b);
                        
                        int const minc1 = cs.r < cs.g ? cs.r : cs.g;
                        int const minc = minc1 < cs.b ? minc1 : cs.b;
                        
                        int const c = maxc - minc;
                                                
                        if( c )
                        {
                                //42 !!!!
                                switch(sel)
                                {
                                        case 1: cd.r = ((int)cs.b - (int)cs.r)*42 / c + 2*42; break;
                                        case 0: cd.r = ((int)cs.g - (int)cs.b)*42 / c; break;       //auto-clamping for free!
                                        default:cd.r = ((int)cs.r - (int)cs.g)*42 / c + 4*42; break;                                        
                                }                                                                                                
                        }
                        else
                                cd.r = 0;
                        
                        if( maxc )                        
                              cd.g = 255 - (((minc << 8) - minc) / maxc);                        
                        else
                                cd.g = 0;
                        
                        cd.b = maxc;                                                        
                        cd.a = cs.a;
                }
        }
        
        return img_dstp;
};

image_pt image_rgba_from_hsva( image_pt const & img_srcp )
{
        image_t const * const img_src = img_srcp.get();
        int const width = img_src->header.width;
        int const height = img_src->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_rgba );
        image_t * const img_dst = img_dstp.get();
        
        
        for( int y = 0; y < height; ++y )
        {
                color4b_t const * const row_src = img_src->row<color4b_t>(y);
                color4b_t * const row_dst = img_dst->row<color4b_t>(y);
                
                for( int x = 0; x < width; ++x )
                {                                  
                        color4b_t const cs = row_src[x];
                        color4b_t & cd = row_dst[x];
                        int const sel = cs.r / 42;
                        int const vmin = (255-cs.g)*cs.b/255;
                        int const a = (cs.b - vmin) * ( cs.a % 42 ) / 42;
                        int const vinc = vmin + a;
                        int const vdec = cs.b - a;
                        
                        switch(sel)
                        {
                                case 0: case 6: cd.r = cs.b; cd.g = vinc; cd.b = vmin; break;
                                case 1: cd.r = vdec; cd.g = cs.b; cd.b = vmin; break;
                                case 2: cd.r = vmin; cd.g = cs.b; cd.b = vinc; break;
                                case 3: cd.r = vmin; cd.g = vdec; cd.b = cs.b; break;
                                case 4: cd.r = vinc; cd.g = vmin; cd.b = cs.b; break;
                                case 5: cd.r = cs.b; cd.g = vmin; cd.b = vdec; break;
                        }                                                
                        cd.a = cs.a;
                }
        }
        
        return img_dstp;
}

static float convert_rgb_xyz( float const c )
{
        if( c <= 0.04045 )
                return c/12.92f;
        else
                return powf( (c + 0.055f) / (1+0.055f), 2.4f );
}

static const float c_xyzlab_point = powf(6.f/29.f,3);
static float convert_xyz_lab( float const c )
{
        float const cx = c;//3.f*c;
        if(cx > c_xyzlab_point)        
                return powf(cx, 1.f/3.f);
        else
                return 1.f/3.f*(29.f/6.f)*(29.f/6.f)*cx + 4.f/29.f;         
}


image_pt image_lab_from_rgba( image_pt const & img_srcp )
{
        image_t const * const img_src = img_srcp.get();
        int const width = img_src->header.width;
        int const height = img_src->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_lab_f32 );
        image_t * const img_dst = img_dstp.get();
        
        for( int y = 0; y < height; ++y )
        {
                color4b_t const * const row_src = img_src->row<color4b_t>(y);
                point3f_t * const row_dst = img_dst->row<point3f_t>(y);
                
                for( int x = 0; x < width; ++x )
                {  
                        color4b_t const cl_src = row_src[x];
                        float const rf = cl_src.r / 255.f, gf = cl_src.g/255.f, bf=cl_src.b/255.f;
                        float const rl = convert_rgb_xyz(rf);
                        float const gl = convert_rgb_xyz(gf);
                        float const bl = convert_rgb_xyz(bf);
                        
                        float const X = 0.4124f*rl + 0.3576f*gl + 0.1805f*bl;
                        float const Y = 0.2126f*rl + 0.7152f*gl + 0.0722f*bl;
                        float const Z = 0.0193f*rl + 0.1192f*gl + 0.9502f*bl;
                        
                        float const L = 116.f*convert_xyz_lab(Y)-16.f;
                        float const a = 500.f*(convert_xyz_lab(X) - convert_xyz_lab(Y));
                        float const b = 200.f*(convert_xyz_lab(Y) - convert_xyz_lab(Z));
                        
                        point3f_t & cd = row_dst[x];
                        cd.x = L; cd.y = a; cd.z = b;                                                
                        
                        if( max_lab.r > L ) max_lab.r = L;
                        if( max_lab.g > a ) max_lab.g = a;
                        if( max_lab.b > b ) max_lab.b = b;
                }
        }
        
        return img_dstp;
}

image_pt image_rgba_from_lab( image_pt const & img_srcp )
{
        //implement me
        return image_pt();
}



}
