#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/utils.hpp>
#include <adapter/image.hpp>    //to save to png file
#include <cmn/image_utils.hpp>
#include <alg/seg_object.hpp>

#include <limits.h>
#include <string>
#include <string.h>
#include <algorithm>

//for tests
#include <adapter/filesystem.hpp>
#include <adapter/plotcirc.hpp>
#include <cmn/name.hpp>
#include <cmn/plotcirc.hpp>



static float const c_grad_tolerance_find = 1;
static float const c_grad_tolerance_flood = 1;
static float const c_grad_sharp_border = 20;          //it's definitely another object!

namespace alg
{
using namespace cmn;

namespace
{
        
struct lowland_t
{
        point2w_t               center;        
        float                   deepness;       //brightness in center
        uint16_t                color;          //0-based
        std::vector<point2w_t>  border;
        std::vector<point2w_t>  border2;          //back-buffer
};
        
struct helper_t
{
        image_pt                img_src;        //rgba input image
        image_pt                img_color;      //uin16_t image, of format_g16 each pixel - indexed color (see below)
        std::vector<lowland_t>  lowlands;
        std::vector<int>        lowlands_by_deepness;       //sorted, indices in lowlands.
        
        uint16_t                color;          //current        
};


} //end of anonymous namespace


static float 
grad( color3f_t const & c1, color3f_t const & c2 ) 
{
        float const cr = c1.r - c2.r;
//         float const cg = c1.g - c2.g;
//         float const cb = c1.b - c2.b;
//         if( (cg*cg + cb*cb) > 100 )
//                 cr = -1000.f;        
        return cr; // - sqrtf(cg*cg + cb*cb);
}

static float
gradt( color3f_t const & c1, color3f_t const & c2 )
{
        return grad(c1,c2) + c_grad_tolerance_find;
}

static float
gradtl( color3f_t const & c1, color3f_t const & c2 )
{
        return grad(c1,c2) + c_grad_tolerance_flood;
}


static void 
ll_examine_neighbors(helper_t & h, image_t * img_color, image_t const * img_src, int const x, int const y)
{
        color3f_t range[3][3];
        float   grads[3][3];      
        color3f_t const * row;
        
        row = img_src->row<color3f_t>( y-1 );        
        range[0][0] = row[x-1]; range[0][1] = row[x]; range[0][2] = row[x+1];
        row = img_src->row<color3f_t>( y );
        range[1][0] = row[x-1]; range[1][1] = row[x]; range[1][2] = row[x+1];
        row = img_src->row<color3f_t>( y+1 );
        range[2][0] = row[x-1]; range[2][1] = row[x]; range[2][2] = row[x+1];
        
        color3f_t const middle = range[1][1];
        grads[0][0] = gradt( middle, range[0][0] ); grads[0][1] = gradt( middle, range[0][1] );  grads[0][2] = gradt( middle, range[0][2] );
        grads[1][0] = gradt( middle, range[1][0] ); grads[1][1] = 0;                             grads[1][2] = gradt( middle, range[1][2] );
        grads[2][0] = gradt( middle, range[2][0] ); grads[2][1] = gradt( middle, range[2][1] );  grads[2][2] = gradt( middle, range[2][2] );
        
        uint16_t source_cl = 0;
        
        for( int gy = 0; gy < 3; ++gy )
        {
                int16_t const iy = y+gy-1;
                uint16_t * const row_dst = img_color->row<uint16_t>(iy);
                
                for( int gx = 0; gx < 3; ++gx)
                {
                        float v = grads[gy][gx];
                        if( v < 0 )
                        {       //grads[gy][gx] is higher than grads[x][y] ... abort                                
                                goto exit_loop;
                        }
                        else
                        {
                                //we have to take a look if the neighbor has a color already
                                uint16_t const cl = row_dst[x+gx-1];
                                if(cl)                                 
                                        source_cl = cl;                                                                
                        }
                }
        }
        
        if( source_cl )
        {
                //we should include ourselves in the current lowland
                lowland_t & ll = h.lowlands[source_cl-1];
                ll.border.push_back(point2w_t(x,y));
                img_color->row<uint16_t>(y)[x] = source_cl;            //write it into the current pixel
                goto exit_loop;
        }

        
        //if we are here this means that we are the topmost element in all neihgborhoods!
        {
                uint16_t cl = ++h.color;
                img_color->row<uint16_t>(y)[x] = cl;
                lowland_t ll;
                ll.center = point2w_t(x,y);
                ll.border.push_back(ll.center);
                ll.color = cl;
                ll.deepness = grad( range[1][1], color3f_t(0,0,0) );
                h.lowlands.push_back( ll );
        }                
exit_loop:;
        
}


static void 
find_lowlands( helper_t & h )
{
        image_header_t const hdr = h.img_src->header;
        image_t const * img_src = h.img_src.get();
        image_t * img_color = h.img_color.get();
        h.lowlands.reserve(16);         //why not ?
        
        for( int y = 1; y < hdr.height-1; ++y )
        {
                for( int x = 1; x < hdr.width-1; ++x )
                {
                        ll_examine_neighbors( h, img_color, img_src, x, y );
                }
        } 

#if 1        
        int n = 0;
        auto inc_range = [&n](){ return n++; };
        h.lowlands_by_deepness.resize( h.lowlands.size() );
        std::generate( h.lowlands_by_deepness.begin(), h.lowlands_by_deepness.end(), inc_range );
                
        std::vector< lowland_t > const & lowlands = h.lowlands;
        auto sort_by_deepness = [&lowlands]( int a, int b )
        {
                return lowlands[a].border.size() > lowlands[b].border.size();
                //return lowlands[a].deepness > lowlands[b].deepness;
        };
                                
        //sort them all
        std::sort( h.lowlands_by_deepness.begin(), h.lowlands_by_deepness.end(), sort_by_deepness );
#endif        
}

static void
grow_contour_examine_neighbors( helper_t & h, lowland_t & ll, image_t * img_color, image_t const * img_src, point2w_t const pt )
{
        color3f_t               range[3][3];
        float                   grads[3][3];        
        color3f_t const *       row;

        int16_t const lastw = img_src->header.width - 1;
        int16_t const lasth = img_src->header.height - 1;
        
        if( pt.y > 0 )
        {
                row = img_src->row<color3f_t>( pt.y-1 );        
                if(pt.x > 0) range[0][0] = row[pt.x-1]; range[0][1] = row[pt.x]; if(pt.x<lastw) range[0][2] = row[pt.x+1];
        }
        row = img_src->row<color3f_t>( pt.y );
        
        if(pt.x > 0) range[1][0] = row[pt.x-1]; range[1][1] = row[pt.x]; if(pt.x<lastw) range[1][2] = row[pt.x+1];
        
        if( pt.y < lasth )
        {
                row = img_src->row<color3f_t>( pt.y+1 );
                if(pt.x > 0) range[2][0] = row[pt.x-1]; range[2][1] = row[pt.x]; if(pt.x<lastw) range[2][2] = row[pt.x+1];
        }

        color3f_t middle = range[1][1];
        grads[0][0] = gradtl( middle, range[0][0] ); grads[0][1] = gradtl( middle, range[0][1] );  grads[0][2] = gradtl( middle, range[0][2] );
        grads[1][0] = gradtl( middle, range[1][0] ); grads[1][1] = 0;                             grads[1][2] = gradtl( middle, range[1][2] );
        grads[2][0] = gradtl( middle, range[2][0] ); grads[2][1] = gradtl( middle, range[2][1] );  grads[2][2] = gradtl( middle, range[2][2] );
                 
        for( int gy = 0; gy < 3; ++gy )
        {
                int16_t const iy = pt.y + gy - 1;
                if( iy < 0 || iy > lasth )
                        continue;
                
                uint16_t * const row_dst = img_color->row<uint16_t>(iy);
                for( int gx = 0; gx < 3; ++gx)
                {
                        int16_t ix = pt.x + gx - 1;                                                
                        if( ix < 0 || ix > lastw )              //out of scope
                                continue;
                        
                        uint16_t color_dst = row_dst[ix];
                        if( !color_dst )        //free
                        {                        
                                float const val = grads[gy][gx];
                                if(val >= 0 && val < c_grad_sharp_border)            //can grow... 
                                {
                                        ll.border2.emplace_back(ix,iy);     
                                        row_dst[ix] = ll.color;
                                }
                        }
                }
        }
}

static bool
grow_contour( helper_t & h, lowland_t & ll )
{
        std::vector<point2w_t> & border = ll.border;        
        int const border_size = (int)border.size();
        image_t const * img_src = h.img_src.get();
        image_t * img_color = h.img_color.get();
        
        for( int i = 0; i != border_size; ++i )
        {
                point2w_t pt = border[i];
                grow_contour_examine_neighbors( h, ll, img_color, img_src, pt );
        }
                
        border.clear();
        border.swap(ll.border2);        
        return !border.empty();
}

static void 
flood( helper_t & h )
{
        //most heavy part - grow from borders until acquire all image!
        //TODO: just a heaven for parallelization
        bool has_points;
        int lowlands_size = (int)h.lowlands.size();
                        
        do
        {
                has_points = false;
                for( int i = 0; i != lowlands_size; ++i )
                {                
                        lowland_t & ll = h.lowlands[i];                        
                        has_points |= grow_contour(h, ll);
                }                             
        } while( has_points );
}

static void 
flood2( helper_t & h )
{
        bool has_points;
        int lowlands_size = (int)h.lowlands_by_deepness.size();
 
        for( int i = 0; i != lowlands_size; ++i )
        {                
                int index = h.lowlands_by_deepness[i];
                lowland_t & ll = h.lowlands[index];
                do
                {                                
                        has_points = grow_contour(h, ll);
                } while( has_points );
        }        
}

static void 
flood3( helper_t & h )
{
        bool has_points;
        int lowlands_size = (int)h.lowlands.size() / 16;
                        
        do
        {
                has_points = false;
                for( int i = 0; i != lowlands_size; ++i )
                {                
                        int index = h.lowlands_by_deepness[i];
                        lowland_t & ll = h.lowlands[index];                        
                        has_points |= grow_contour(h, ll);
                }                             
        } while( has_points );     
}

void watershed2( std::vector< seg_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img_lab )
{
        helper_t h;
        int const width = img_lab->header.width;
        int const height = img_lab->header.height;
        
        h.img_src = img_lab;
        h.img_color = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g16 );
        memset( h.img_color->bytes, 0, height * h.img_color->header.pitch );
        h.color = 0;
        
        find_lowlands(h);
        
        memset( h.img_color->bytes, 0, height * h.img_color->header.pitch );
        flood3(h);

        if( objects )
                seg_create_objects( objects, h.color, h.img_color );
        
        if( colored )
                *colored = seg_color( h.img_color );

}


static void 
watershed_test_do_rest( cmn::image_pt const & img, std::string const & base_name )
{
        std::string dir_dst = adapter::fs_prefs_dir();
        cmn::image_pt img_painted;
        watershed2( 0, &img_painted, img );
        adapter::image_save_to_png( img_painted, (dir_dst + base_name + ".png").c_str() );
}

void watershed2_test()
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/faces_png/";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects/";                
        adapter::fs_make_dir( dir_objects );                

        std::vector<adapter::fs_file_info_t> files;
        adapter::fs_dir_contents( &files, dir_src );
        
        adapter::fs_file_info_t fi1;
        fi1.name = "c.png";
        //files.push_back(fi1);
        
        for( size_t i = 0; i < files.size(); ++i )
        {        
                adapter::fs_file_info_t const & fi = files[i];
                
                cmn::image_pt img = adapter::image_create_from_png( (dir_src + fi.name).c_str() );        
                cmn::image_pt img_lab = cmn::image_lab_from_rgba( img );
                
                std::string base_name = "objects/" + adapter::fs_name_ext( fi.name ).first;
                
                //sobel_test_do_rest(img, base_name);
                watershed_test_do_rest(img_lab, base_name+"_lab");   
        }
}


} //alg


